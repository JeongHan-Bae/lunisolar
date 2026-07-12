#include <iomanip>
#include <iostream>
#include <optional>
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

    [[nodiscard]] std::optional<lunisolar::gregorian_date> birthday_in_chinese_year(
            lunisolar::gregorian_date original_birthday,
            int chinese_year,
            bool fallback_invalid_leap_month
    ) {
        const auto original_chinese_birthday = Calendar::from_gregorian(original_birthday);
        if (!original_chinese_birthday) {
            return std::nullopt;
        }

        const lunisolar::chinese_date birthday_in_year{
                chinese_year,
                original_chinese_birthday.value.month,
                original_chinese_birthday.value.day,
                original_chinese_birthday.value.leap
        };

        const auto gregorian_birthday = Calendar::to_gregorian(birthday_in_year);
        if (gregorian_birthday) {
            return gregorian_birthday.value;
        }

        if (!fallback_invalid_leap_month ||
            !original_chinese_birthday.value.leap ||
            gregorian_birthday.ec != lunisolar::error::invalid_leap_month) {
            return std::nullopt;
        }

        const lunisolar::chinese_date fallback_birthday{
                chinese_year,
                original_chinese_birthday.value.month,
                original_chinese_birthday.value.day,
                false
        };

        const auto fallback_gregorian_birthday = Calendar::to_gregorian(fallback_birthday);
        if (!fallback_gregorian_birthday) {
            return std::nullopt;
        }

        return fallback_gregorian_birthday.value;
    }

    [[nodiscard]] std::optional<std::string> three_pillars_text(
            lunisolar::gregorian_date date,
            lunisolar::month_basis basis
    ) {
        const auto pillars = Calendar::simplified_four_pillars(
                date,
                0U,
                lunisolar::day_boundary::midnight,
                basis
        );
        if (!pillars) {
            return std::nullopt;
        }

        return lunisolar::to_string(pillars.value.year, lunisolar::text_style::hant_cn) +
               "年 " +
               lunisolar::to_string(pillars.value.month, lunisolar::text_style::hant_cn) +
               "月 " +
               lunisolar::to_string(pillars.value.day, lunisolar::text_style::hant_cn) +
               "日";
    }

    bool print_gregorian_birthdays_for_chinese_years(
            lunisolar::gregorian_date original_birthday,
            int first_chinese_year,
            int last_chinese_year,
            bool fallback_invalid_leap_month
    ) {
        const auto original_chinese_birthday = Calendar::from_gregorian(original_birthday);
        if (!original_chinese_birthday) {
            std::cerr << "Failed to convert the original Gregorian birthday.\n";
            return false;
        }

        std::cout << "Author Gregorian birthday: ";
        print_gregorian_date(original_birthday);
        std::cout << '\n';

        std::cout << "Author Chinese birthday: year "
                  << original_chinese_birthday.value.year
                  << ", month "
                  << original_chinese_birthday.value.month
                  << ", day "
                  << original_chinese_birthday.value.day;

        if (original_chinese_birthday.value.leap) {
            std::cout << " (leap month)";
        }

        std::cout << "\n";

        const auto nearest_term = Calendar::nearest_solar_term(original_birthday);
        if (!nearest_term) {
            std::cerr << "Failed to resolve nearest solar term for author's birthday.\n";
            return false;
        }

        std::cout << "Nearest solar term for Author's birthday: "
                  << lunisolar::to_string(nearest_term.value.term, lunisolar::text_style::hant_cn)
                  << " -> ";
        print_gregorian_date(nearest_term.value.date);
        std::cout << "\n\n";

        std::cout << "Gregorian birthdays for Chinese years "
                  << first_chinese_year
                  << " to "
                  << last_chinese_year
                  << ":\n";

        for (int chinese_year = first_chinese_year;
             chinese_year <= last_chinese_year;
             ++chinese_year) {
            const auto gregorian_birthday = birthday_in_chinese_year(
                    original_birthday,
                    chinese_year,
                    fallback_invalid_leap_month
            );

            if (!gregorian_birthday) {
                std::cerr << "Failed to convert Chinese year " << chinese_year << ".\n";
                return false;
            }

            std::cout << "Chinese year " << chinese_year << " -> ";
            print_gregorian_date(*gregorian_birthday);

            const auto solar_term_pillars = three_pillars_text(
                    *gregorian_birthday,
                    lunisolar::month_basis::solar_term_month
            );
            if (!solar_term_pillars) {
                std::cerr << " Failed to compute the solar-term-based three pillars.\n";
                return false;
            }

            const auto lunar_new_year_pillars = three_pillars_text(
                    *gregorian_birthday,
                    lunisolar::month_basis::lunar_month
            );
            if (!lunar_new_year_pillars) {
                std::cerr << " Failed to compute the Chinese-New-Year-based three pillars.\n";
                return false;
            }

            std::cout << " Lichun: "
                      << *solar_term_pillars
                      << " New Year: "
                      << *lunar_new_year_pillars;

            if (original_chinese_birthday.value.leap &&
                fallback_invalid_leap_month &&
                Calendar::leap_month(chinese_year) != original_chinese_birthday.value.month) {
                std::cout << " (fallback to non-leap month)";
            }

            std::cout << '\n';
        }

        return true;
    }

}

int main() {
    constexpr lunisolar::gregorian_date author_birthday{2001, 1, 20};

    if (!print_gregorian_birthdays_for_chinese_years(
                author_birthday,
                2000,
                2100,
                true
        )) {
        return 1;
    }

    return 0;
}
