#include <iomanip>
#include <iostream>
#include <optional>
#include <random>
#include <string>

#include "lunisolar.h"
#include "lunisolar_data.h"

using Calendar = lunisolar::data::calendar;

namespace {

    void print_gregorian_date(const lunisolar::gregorian_date &date) {
        std::cout << std::setfill('0')
                  << std::setw(4) << date.year
                  << '-'
                  << std::setw(2) << date.month
                  << '-'
                  << std::setw(2) << date.day
                  << std::setfill(' ');
    }

    [[nodiscard]] std::optional<std::string> approximate_moon_phase_text(
            lunisolar::gregorian_date date
    ) {
        const auto chinese_date = Calendar::from_gregorian(date);
        if (!chinese_date) {
            return std::nullopt;
        }

        const auto month_days = Calendar::regular_month_days(
                chinese_date.value.year,
                chinese_date.value.month,
                chinese_date.value.leap
        );
        if (month_days <= 1U) {
            return std::nullopt;
        }

        const double progress =
                static_cast<double>(chinese_date.value.day - 1U) /
                static_cast<double>(month_days - 1U);

        if (progress <= 0.0625) {
            return "朔";
        }
        if (progress <= 0.1875) {
            return "娥眉月";
        }
        if (progress <= 0.3125) {
            return "上弦月";
        }
        if (progress <= 0.4375) {
            return "盈凸月";
        }
        if (progress <= 0.5625) {
            return "望";
        }
        if (progress <= 0.6875) {
            return "虧凸月";
        }
        if (progress <= 0.8125) {
            return "下弦月";
        }
        if (progress <= 0.9375) {
            return "殘月";
        }
        return "晦";
    }

    [[nodiscard]] std::optional<lunisolar::gregorian_date> lichun_of_year(int gregorian_year) {
        const auto lichun = Calendar::solar_term_date(gregorian_year, lunisolar::solar_term::lichun);
        if (!lichun) {
            return std::nullopt;
        }
        return lichun.value;
    }

    [[nodiscard]] lunisolar::gregorian_date random_supported_date(std::mt19937_64 &rng) {
        const auto first = lunisolar::detail::to_sys_days(Calendar::first_gregorian_date());
        const auto last = lunisolar::detail::to_sys_days(Calendar::last_gregorian_date());
        const auto span = static_cast<unsigned long long>((last - first).count());

        std::uniform_int_distribution<unsigned long long> distribution{0ULL, span};
        return lunisolar::detail::from_sys_days(first + std::chrono::days{distribution(rng)});
    }

}

int main() {
    constexpr std::uint64_t seed = 20010120ULL;
    constexpr int sample_count = 8;

    std::mt19937_64 rng{seed};

    std::cout << "Moon phase and solar term examples:\n";
    std::cout << "Random sample seed: " << seed << '\n';

    for (int i = 0; i < sample_count; ++i) {
        const auto sample_date = random_supported_date(rng);

        const auto chinese_date = Calendar::from_gregorian(sample_date);
        if (!chinese_date) {
            std::cerr << "Failed to convert a random Gregorian date.\n";
            return 1;
        }

        const auto month_days = Calendar::regular_month_days(
                chinese_date.value.year,
                chinese_date.value.month,
                chinese_date.value.leap
        );

        const auto moon_phase = approximate_moon_phase_text(sample_date);
        if (!moon_phase) {
            std::cerr << "Failed to estimate the moon phase.\n";
            return 1;
        }

        const auto previous_term = Calendar::previous_solar_term(sample_date);
        if (!previous_term) {
            std::cerr << "Failed to resolve the previous solar term.\n";
            return 1;
        }

        const auto nearest_term = Calendar::nearest_solar_term(sample_date);
        if (!nearest_term) {
            std::cerr << "Failed to resolve the nearest solar term.\n";
            return 1;
        }

        const auto next_term = Calendar::next_solar_term(sample_date);
        if (!next_term) {
            std::cerr << "Failed to resolve the next solar term.\n";
            return 1;
        }

        const auto lichun = lichun_of_year(sample_date.year);
        if (!lichun) {
            std::cerr << "Failed to resolve the year's Li-Chun.\n";
            return 1;
        }

        std::cout << '\n' << "Sample " << (i + 1) << ":\n";
        std::cout << "Date: ";
        print_gregorian_date(sample_date);
        std::cout << '\n';

        std::cout << "Chinese date: year "
                  << chinese_date.value.year
                  << ", month "
                  << chinese_date.value.month
                  << ", day "
                  << chinese_date.value.day;
        if (chinese_date.value.leap) {
            std::cout << " (leap month)";
        }
        std::cout << '\n';

        std::cout << "Approximate moon phase: "
                  << *moon_phase
                  << " ("
                  << chinese_date.value.day
                  << "/"
                  << month_days
                  << ")\n";

        std::cout << "Nearest solar term: "
                  << lunisolar::to_string(nearest_term.value.term, lunisolar::text_style::hant_cn)
                  << " on ";
        print_gregorian_date(nearest_term.value.date);
        std::cout << '\n';

        std::cout << "Previous solar term: "
                  << lunisolar::to_string(previous_term.value.term, lunisolar::text_style::hant_cn)
                  << " on ";
        print_gregorian_date(previous_term.value.date);
        std::cout << '\n';

        std::cout << "Next solar term: "
                  << lunisolar::to_string(next_term.value.term, lunisolar::text_style::hant_cn)
                  << " on ";
        print_gregorian_date(next_term.value.date);
        std::cout << '\n';

        std::cout << "Li-Chun of Gregorian year " << sample_date.year << ": ";
        print_gregorian_date(*lichun);
        std::cout << '\n';
    }

    return 0;
}
