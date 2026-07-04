#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <tuple>
#include <type_traits>

#include "lunisolar.h"
#include "lunisolar_data.h"

using Calendar = lunisolar::data::calendar;

namespace {

    struct known_date_vector {
        int solar_year;
        unsigned solar_month;
        unsigned solar_day;
        int lunar_year;
        unsigned lunar_month;
        unsigned lunar_day;
        bool leap;
    };

    constexpr std::size_t known_date_vector_count =
#include "lunisolar_vectors_count.inc"
            ;

    constexpr std::array<known_date_vector, known_date_vector_count> known_date_vectors{{
#include "lunisolar_vectors.inc"
    }};

    [[nodiscard]] constexpr unsigned physical_month_count(unsigned leap_month) noexcept {
        return leap_month == 0 ? 12U : 13U;
    }

    [[nodiscard]] constexpr unsigned physical_month_days(
            std::uint16_t month_bits,
            unsigned physical_index
    ) noexcept {
        return ((month_bits >> physical_index) & 1U) != 0U ? 30U : 29U;
    }

    void test_packed_data_layout() {
        static_assert(Calendar::supported_year_count() ==
                      lunisolar::supported_year_count<
                              lunisolar::data::first_supported_year,
                              lunisolar::data::last_supported_year
                      >());

        static_assert(Calendar::first_supported_year ==
                      lunisolar::data::first_supported_year);

        static_assert(Calendar::last_supported_year ==
                      lunisolar::data::last_supported_year);

        static_assert(Calendar::day_ganzhi_anchor_date ==
                      lunisolar::data::day_ganzhi_anchor_date);

        static_assert(Calendar::day_ganzhi_anchor_index ==
                      lunisolar::data::day_ganzhi_anchor_index % 60U);

        static_assert(lunisolar::solar_term_count == 24);
        static_assert(lunisolar::packed_solar_term_word_count == 2);

        static_assert(lunisolar::data::packed_years.size() ==
                      Calendar::supported_year_count());

        static_assert(lunisolar::data::packed_solar_terms.size() ==
                      Calendar::supported_year_count());

        for (std::size_t i = 0; i < Calendar::supported_year_count(); ++i) {
            const auto packed = lunisolar::data::packed_years[i];

            const auto month_bits = lunisolar::detail::packed_month_bits(packed);
            const auto leap_month = lunisolar::detail::packed_leap_month(packed);
            const auto new_year_day = lunisolar::detail::packed_new_year_day(packed);
            const auto year_length = lunisolar::detail::packed_year_length(packed);

            assert(leap_month <= 12);
            assert(new_year_day <= 60);
            assert(year_length >= 353 && year_length <= 385);

            unsigned sum = 0;
            const unsigned count = physical_month_count(leap_month);

            for (unsigned physical = 0; physical < count; ++physical) {
                const unsigned days = physical_month_days(month_bits, physical);
                assert(days == 29 || days == 30);
                sum += days;
            }

            assert(sum == year_length);

            if (leap_month == 0) {
                assert(((month_bits >> 12U) & 1U) == 0U);
            }
        }
    }

    void test_calendar_matches_packed_data() {
        for (int year = Calendar::first_supported_year;
             year <= Calendar::last_supported_year;
             ++year) {
            const auto packed = Calendar::packed_year(year);

            const auto month_bits = lunisolar::detail::packed_month_bits(packed);
            const auto leap_month = lunisolar::detail::packed_leap_month(packed);
            const auto new_year_day = lunisolar::detail::packed_new_year_day(packed);
            const auto year_length = lunisolar::detail::packed_year_length(packed);

            assert(Calendar::leap_month(year) == leap_month);
            assert(Calendar::physical_month_count(year) == physical_month_count(leap_month));
            assert(Calendar::year_length(year) == year_length);

            const auto cny = Calendar::chinese_new_year(year);
            const auto cny_date = lunisolar::detail::from_sys_days(cny);

            assert(cny_date.year == year);
            assert(lunisolar::detail::day_of_year_zero_based(cny_date) == new_year_day);

            unsigned sum = 0;
            for (unsigned physical = 0;
                 physical < Calendar::physical_month_count(year);
                 ++physical) {
                const unsigned expected = physical_month_days(month_bits, physical);
                const unsigned actual = Calendar::physical_month_days(year, physical);

                assert(actual == expected);
                assert(actual == 29 || actual == 30);

                sum += actual;
            }

            assert(sum == year_length);
        }
    }

    void test_full_round_trip_every_supported_day() {
        const auto first =
                lunisolar::detail::to_sys_days(Calendar::first_gregorian_date());

        const auto last =
                lunisolar::detail::to_sys_days(Calendar::last_gregorian_date());

        std::uint64_t checked_days = 0;

        for (auto day = first; day <= last; day += std::chrono::days{1}) {
            const auto gregorian = lunisolar::detail::from_sys_days(day);

            const auto chinese = Calendar::from_gregorian(gregorian);
            assert(chinese);
            assert(Calendar::valid(chinese.value));

            const auto back = Calendar::to_gregorian(chinese.value);
            assert(back);
            assert(back.value == gregorian);

            ++checked_days;
        }

        std::uint64_t expected_days = 0;
        for (int year = Calendar::first_supported_year;
             year <= Calendar::last_supported_year;
             ++year) {
            expected_days += Calendar::year_length(year);
        }

        assert(checked_days == expected_days);
    }

    void test_boundaries() {
        static_assert(Calendar::first_supported_year ==
                      lunisolar::data::first_supported_year);

        static_assert(Calendar::last_supported_year ==
                      lunisolar::data::last_supported_year);

        static_assert(Calendar::supported_year_count() ==
                      lunisolar::supported_year_count<
                              Calendar::first_supported_year,
                              Calendar::last_supported_year
                      >());

        static_assert(Calendar::supports_chinese_year(Calendar::first_supported_year));
        static_assert(Calendar::supports_chinese_year(Calendar::last_supported_year));
        static_assert(!Calendar::supports_chinese_year(Calendar::first_supported_year - 1));
        static_assert(!Calendar::supports_chinese_year(Calendar::last_supported_year + 1));

        {
            const auto first = Calendar::first_gregorian_date();
            const auto result = Calendar::from_gregorian(first);

            assert(result);
            assert(result.value.year == Calendar::first_supported_year);
            assert(result.value.month == 1);
            assert(result.value.day == 1);
            assert(!result.value.leap);
        }

        {
            const auto last = Calendar::last_gregorian_date();
            const auto result = Calendar::from_gregorian(last);

            assert(result);
            assert(result.value.year == Calendar::last_supported_year);
        }

        {
            const auto before_first =
                    lunisolar::detail::from_sys_days(
                            lunisolar::detail::to_sys_days(Calendar::first_gregorian_date()) -
                            std::chrono::days{1}
                    );

            const auto result = Calendar::from_gregorian(before_first);
            assert(!result);
            assert(result.ec == lunisolar::error::year_out_of_range);
        }

        {
            const auto after_last =
                    lunisolar::detail::from_sys_days(
                            lunisolar::detail::to_sys_days(Calendar::last_gregorian_date()) +
                            std::chrono::days{1}
                    );

            const auto result = Calendar::from_gregorian(after_last);
            assert(!result);
            assert(result.ec == lunisolar::error::year_out_of_range);
        }
    }

    void test_invalid_inputs() {
        {
            const auto result = Calendar::from_gregorian({2024, 2, 30});
            assert(!result);
            assert(result.ec == lunisolar::error::invalid_gregorian_date);
        }

        {
            const auto result = Calendar::to_gregorian({
                                                               Calendar::first_supported_year - 1,
                                                               1,
                                                               1,
                                                               false
                                                       });
            assert(!result);
            assert(result.ec == lunisolar::error::year_out_of_range);
        }

        {
            const auto result = Calendar::to_gregorian({
                                                               Calendar::last_supported_year + 1,
                                                               1,
                                                               1,
                                                               false
                                                       });
            assert(!result);
            assert(result.ec == lunisolar::error::year_out_of_range);
        }

        if (Calendar::supports_chinese_year(2024)) {
            {
                const auto result = Calendar::to_gregorian({2024, 0, 1, false});
                assert(!result);
                assert(result.ec == lunisolar::error::invalid_chinese_month);
            }

            {
                const auto result = Calendar::to_gregorian({2024, 13, 1, false});
                assert(!result);
                assert(result.ec == lunisolar::error::invalid_chinese_month);
            }

            {
                const auto result = Calendar::to_gregorian({2024, 1, 31, false});
                assert(!result);
                assert(result.ec == lunisolar::error::invalid_chinese_day);
            }
        }

        if (Calendar::supports_chinese_year(2021)) {
            const auto result = Calendar::to_gregorian({2021, 4, 1, true});
            assert(!result);
            assert(result.ec == lunisolar::error::invalid_leap_month);
        }
    }

    void test_known_dates() {
        for (const auto &vector : known_date_vectors) {
            const lunisolar::gregorian_date solar{
                    vector.solar_year,
                    vector.solar_month,
                    vector.solar_day
            };
            const lunisolar::chinese_date lunar{
                    vector.lunar_year,
                    vector.lunar_month,
                    vector.lunar_day,
                    vector.leap
            };

            const auto from = Calendar::from_gregorian(solar);
            assert(from);
            assert(from.value == lunar);

            const auto to = Calendar::to_gregorian(lunar);
            assert(to);
            assert(to.value == solar);
        }
    }

    void test_solar_term_offsets() {
        for (int year = Calendar::first_supported_year;
             year <= Calendar::last_supported_year;
             ++year) {
            const auto index = static_cast<std::size_t>(
                    year - Calendar::first_supported_year
            );

            for (unsigned i = 0; i < lunisolar::solar_term_count; ++i) {
                const auto term = static_cast<lunisolar::solar_term>(i);
                const auto result = Calendar::solar_term_date(year, term);

                assert(result);
                assert(result.value.year == year);

                const int offset = lunisolar::detail::unpack_solar_term_offset(
                        lunisolar::data::packed_solar_terms[index],
                        i
                );

                assert(offset >= -8);
                assert(offset <= 7);

                const auto base =
                        std::chrono::sys_days{
                                std::chrono::year{year} /
                                std::chrono::month{lunisolar::detail::solar_term_base_month[i]} /
                                std::chrono::day{lunisolar::detail::solar_term_base_day[i]}
                        };

                const auto actual = lunisolar::detail::to_sys_days(result.value);
                assert((actual - base).count() == offset);
            }
        }
    }

    void test_known_solar_terms() {
        if (!Calendar::supports_gregorian_year(2024)) {
            return;
        }

        {
            const auto lichun = Calendar::solar_term_date(
                    2024,
                    lunisolar::solar_term::lichun
            );

            assert(lichun);
            assert(lichun.value.year == 2024);
            assert(lichun.value.month == 2);
            assert(lichun.value.day >= 3);
            assert(lichun.value.day <= 5);
        }

        {
            const auto chunfen = Calendar::solar_term_date(
                    2024,
                    lunisolar::solar_term::chunfen
            );

            assert(chunfen);
            assert(chunfen.value.year == 2024);
            assert(chunfen.value.month == 3);
            assert(chunfen.value.day >= 19);
            assert(chunfen.value.day <= 22);
        }

        {
            const auto xiazhi = Calendar::solar_term_date(
                    2024,
                    lunisolar::solar_term::xiazhi
            );

            assert(xiazhi);
            assert(xiazhi.value.year == 2024);
            assert(xiazhi.value.month == 6);
            assert(xiazhi.value.day >= 20);
            assert(xiazhi.value.day <= 22);
        }

        {
            const auto qiufen = Calendar::solar_term_date(
                    2024,
                    lunisolar::solar_term::qiufen
            );

            assert(qiufen);
            assert(qiufen.value.year == 2024);
            assert(qiufen.value.month == 9);
            assert(qiufen.value.day >= 22);
            assert(qiufen.value.day <= 24);
        }

        {
            const auto dongzhi = Calendar::solar_term_date(
                    2024,
                    lunisolar::solar_term::dongzhi
            );

            assert(dongzhi);
            assert(dongzhi.value.year == 2024);
            assert(dongzhi.value.month == 12);
            assert(dongzhi.value.day >= 20);
            assert(dongzhi.value.day <= 23);
        }
    }

    void test_solar_term_year_month() {
        if (!Calendar::supports_gregorian_year(2024) ||
            !Calendar::supports_chinese_year(2023)) {
            return;
        }

        {
            const auto lichun = Calendar::solar_term_date(
                    2024,
                    lunisolar::solar_term::lichun
            );
            assert(lichun);

            const auto before_lichun = lunisolar::detail::from_sys_days(
                    lunisolar::detail::to_sys_days(lichun.value) -
                    std::chrono::days{1}
            );

            const auto at_lichun = lichun.value;

            const auto before = Calendar::solar_term_year_month_of(before_lichun);
            assert(before);
            assert(before.value.year == 2023);
            assert(before.value.month == 12);

            const auto at = Calendar::solar_term_year_month_of(at_lichun);
            assert(at);
            assert(at.value.year == 2024);
            assert(at.value.month == 1);
        }

        {
            const auto jingzhe = Calendar::solar_term_date(
                    2024,
                    lunisolar::solar_term::jingzhe
            );
            assert(jingzhe);

            const auto before_jingzhe = lunisolar::detail::from_sys_days(
                    lunisolar::detail::to_sys_days(jingzhe.value) -
                    std::chrono::days{1}
            );

            const auto before = Calendar::solar_term_year_month_of(before_jingzhe);
            assert(before);
            assert(before.value.year == 2024);
            assert(before.value.month == 1);

            const auto at = Calendar::solar_term_year_month_of(jingzhe.value);
            assert(at);
            assert(at.value.year == 2024);
            assert(at.value.month == 2);
        }

        {
            const auto daxue = Calendar::solar_term_date(
                    2024,
                    lunisolar::solar_term::daxue
            );
            assert(daxue);

            const auto at = Calendar::solar_term_year_month_of(daxue.value);
            assert(at);
            assert(at.value.year == 2024);
            assert(at.value.month == 11);
        }
    }

    void test_ganzhi() {
        {
            const auto gz = Calendar::year_ganzhi(1984);
            assert(gz.index == 0);
            assert(lunisolar::to_string(gz, lunisolar::text_style::hans_cn) == "甲子");
        }

        {
            const auto gz = Calendar::year_ganzhi(2024);
            assert(lunisolar::to_string(gz, lunisolar::text_style::hans_cn) == "甲辰");
            assert(lunisolar::to_string(gz, lunisolar::text_style::pinyin) == "jia-chen");
            assert(lunisolar::to_string(gz, lunisolar::text_style::eon_mun_ko_cn) == "갑진");
        }

        {
            const auto gz = Calendar::day_ganzhi(Calendar::day_ganzhi_anchor_date);
            assert(gz.index == Calendar::day_ganzhi_anchor_index);
        }

        {
            const auto gz = Calendar::day_ganzhi({2019, 1, 27});
            assert(gz.index == 0);
            assert(lunisolar::to_string(gz, lunisolar::text_style::hans_cn) == "甲子");
        }

        {
            const auto gz = Calendar::hour_ganzhi({2019, 1, 27}, 23);
            assert(lunisolar::to_string(gz, lunisolar::text_style::hans_cn) == "甲子");
        }

        {
            const auto gz = Calendar::hour_ganzhi(
                    {2019, 1, 27},
                    23,
                    lunisolar::day_boundary::zi_hour
            );
            assert(lunisolar::to_string(gz, lunisolar::text_style::hans_cn) == "丙子");
        }
    }

    void test_text_mappings() {
        assert(lunisolar::to_string(
                lunisolar::heavenly_stem::jia,
                lunisolar::text_style::hans_cn
        ) == "甲");

        assert(lunisolar::to_string(
                lunisolar::heavenly_stem::jia,
                lunisolar::text_style::hant_cn
        ) == "甲");

        assert(lunisolar::to_string(
                lunisolar::heavenly_stem::jia,
                lunisolar::text_style::pinyin
        ) == "jia");

        assert(lunisolar::to_string(
                lunisolar::heavenly_stem::jia,
                lunisolar::text_style::eon_mun_ko_cn
        ) == "갑");

        assert(lunisolar::to_string(
                lunisolar::heavenly_stem::jia,
                lunisolar::text_style::numeric
        ) == "0");

        assert(lunisolar::to_string(
                lunisolar::earthly_branch::zi,
                lunisolar::text_style::hans_cn
        ) == "子");

        assert(lunisolar::to_string(
                lunisolar::earthly_branch::zi,
                lunisolar::text_style::hant_cn
        ) == "子");

        assert(lunisolar::to_string(
                lunisolar::earthly_branch::zi,
                lunisolar::text_style::pinyin
        ) == "zi");

        assert(lunisolar::to_string(
                lunisolar::earthly_branch::zi,
                lunisolar::text_style::eon_mun_ko_cn
        ) == "자");

        assert(lunisolar::to_string(
                lunisolar::earthly_branch::zi,
                lunisolar::text_style::numeric
        ) == "0");

        assert(lunisolar::to_string(
                lunisolar::ganzhi{0},
                lunisolar::text_style::hans_cn
        ) == "甲子");

        assert(lunisolar::to_string(
                lunisolar::ganzhi{0},
                lunisolar::text_style::hant_cn
        ) == "甲子");

        assert(lunisolar::to_string(
                lunisolar::ganzhi{0},
                lunisolar::text_style::pinyin
        ) == "jia-zi");

        assert(lunisolar::to_string(
                lunisolar::ganzhi{0},
                lunisolar::text_style::eon_mun_ko_cn
        ) == "갑자");

        assert(lunisolar::to_string(
                lunisolar::ganzhi{0},
                lunisolar::text_style::numeric
        ) == "0");
    }

    void test_solar_term_text_mappings() {
        assert(lunisolar::to_string(
                lunisolar::solar_term::lichun,
                lunisolar::text_style::hans_cn
        ) == "立春");

        assert(lunisolar::to_string(
                lunisolar::solar_term::lichun,
                lunisolar::text_style::hant_cn
        ) == "立春");

        assert(lunisolar::to_string(
                lunisolar::solar_term::lichun,
                lunisolar::text_style::pinyin
        ) == "lichun");

        assert(lunisolar::to_string(
                lunisolar::solar_term::lichun,
                lunisolar::text_style::eon_mun_ko_cn
        ) == "립춘");

        assert(lunisolar::to_string(
                lunisolar::solar_term::lixia,
                lunisolar::text_style::eon_mun_ko_cn
        ) == "립하");

        assert(lunisolar::to_string(
                lunisolar::solar_term::liqiu,
                lunisolar::text_style::eon_mun_ko_cn
        ) == "립추");

        assert(lunisolar::to_string(
                lunisolar::solar_term::lidong,
                lunisolar::text_style::eon_mun_ko_cn
        ) == "립동");

        assert(lunisolar::to_string(
                lunisolar::solar_term::jingzhe,
                lunisolar::text_style::hans_cn
        ) == "惊蛰");

        assert(lunisolar::to_string(
                lunisolar::solar_term::jingzhe,
                lunisolar::text_style::hant_cn
        ) == "驚蟄");

        assert(lunisolar::to_string(
                lunisolar::solar_term::guyu,
                lunisolar::text_style::hans_cn
        ) == "谷雨");

        assert(lunisolar::to_string(
                lunisolar::solar_term::guyu,
                lunisolar::text_style::hant_cn
        ) == "穀雨");

        assert(lunisolar::to_string(
                lunisolar::solar_term::xiaoman,
                lunisolar::text_style::hans_cn
        ) == "小满");

        assert(lunisolar::to_string(
                lunisolar::solar_term::xiaoman,
                lunisolar::text_style::hant_cn
        ) == "小滿");

        assert(lunisolar::to_string(
                lunisolar::solar_term::mangzhong,
                lunisolar::text_style::hans_cn
        ) == "芒种");

        assert(lunisolar::to_string(
                lunisolar::solar_term::mangzhong,
                lunisolar::text_style::hant_cn
        ) == "芒種");

        assert(lunisolar::to_string(
                lunisolar::solar_term::chushu,
                lunisolar::text_style::hans_cn
        ) == "处暑");

        assert(lunisolar::to_string(
                lunisolar::solar_term::chushu,
                lunisolar::text_style::hant_cn
        ) == "處暑");

        assert(lunisolar::to_string(
                lunisolar::solar_term::dahan,
                lunisolar::text_style::numeric
        ) == "23");
    }

    void test_hour_ganzhi_with_offset() {
        if (!Calendar::supports_gregorian_year(2024)) {
            return;
        }

        using namespace std::chrono;

        const sys_seconds utc{
                sys_days{year{2024} / February / day{9}} + hours{15}
        };

        {
            const auto local = lunisolar::local_parts(utc, hours{8});
            assert((local.date == lunisolar::gregorian_date{2024, 2, 9}));
            assert(local.hour == 23);
            assert(local.minute == 0);
            assert(local.second == 0);
        }

        {
            const auto midnight_boundary = Calendar::checked_hour_ganzhi(
                    utc,
                    hours{8},
                    lunisolar::day_boundary::midnight
            );

            const auto zi_boundary = Calendar::checked_hour_ganzhi(
                    utc,
                    hours{8},
                    lunisolar::day_boundary::zi_hour
            );

            assert(midnight_boundary);
            assert(zi_boundary);

            assert(midnight_boundary.value != zi_boundary.value);
        }

        {
            const auto local = lunisolar::local_parts(
                    utc,
                    seconds{8 * 3600 + 21 * 60}
            );

            assert((local.date == lunisolar::gregorian_date{2024, 2, 9}));
            assert(local.hour == 23);
            assert(local.minute == 21);
            assert(local.second == 0);

            const auto gz = Calendar::checked_hour_ganzhi(
                    utc,
                    seconds{8 * 3600 + 21 * 60},
                    lunisolar::day_boundary::zi_hour
            );

            assert(gz);
        }

        {
            const sys_seconds utc2{
                    sys_days{year{2024} / February / day{9}} +
                    hours{15} +
                    minutes{40}
            };

            const auto local = lunisolar::local_parts(
                    utc2,
                    seconds{8 * 3600 + 21 * 60}
            );

            assert((local.date == lunisolar::gregorian_date{2024, 2, 10}));
            assert(local.hour == 0);
            assert(local.minute == 1);
            assert(local.second == 0);

            const auto by_midnight = Calendar::checked_hour_ganzhi(
                    utc2,
                    seconds{8 * 3600 + 21 * 60},
                    lunisolar::day_boundary::midnight
            );

            const auto by_zi_hour = Calendar::checked_hour_ganzhi(
                    utc2,
                    seconds{8 * 3600 + 21 * 60},
                    lunisolar::day_boundary::zi_hour
            );

            assert(by_midnight);
            assert(by_zi_hour);

            assert(by_midnight.value == by_zi_hour.value);
        }
    }

    void test_simplified_four_pillars() {
        if (!Calendar::supports_gregorian_year(2024) ||
            !Calendar::supports_chinese_year(2024)) {
            return;
        }

        using namespace std::chrono;

        {
            const auto fp = Calendar::simplified_four_pillars(
                    lunisolar::gregorian_date{2024, 2, 10},
                    0,
                    lunisolar::day_boundary::midnight,
                    lunisolar::month_basis::solar_term_month
            );

            assert(fp);

            assert(lunisolar::to_string(fp.value.year, lunisolar::text_style::hans_cn) == "甲辰");
            assert(lunisolar::to_string(fp.value.month, lunisolar::text_style::hans_cn) == "丙寅");
            assert(lunisolar::to_string(fp.value.hour, lunisolar::text_style::hans_cn).ends_with("子"));

            const auto text = lunisolar::join(fp.value, lunisolar::text_style::hans_cn);
            assert(!text.empty());
        }

        {
            const auto fp = Calendar::simplified_four_pillars(
                    lunisolar::gregorian_date{2024, 2, 10},
                    0,
                    lunisolar::day_boundary::midnight,
                    lunisolar::month_basis::lunar_month
            );

            assert(fp);

            assert(lunisolar::to_string(fp.value.year, lunisolar::text_style::hans_cn) == "甲辰");
            assert(lunisolar::to_string(fp.value.month, lunisolar::text_style::hans_cn) == "丙寅");
        }

        {
            const sys_seconds utc{
                    sys_days{year{2024} / February / day{9}} +
                    hours{15}
            };

            const auto fp_midnight = Calendar::simplified_four_pillars(
                    utc,
                    hours{8},
                    lunisolar::day_boundary::midnight,
                    lunisolar::month_basis::solar_term_month
            );

            const auto fp_zi = Calendar::simplified_four_pillars(
                    utc,
                    hours{8},
                    lunisolar::day_boundary::zi_hour,
                    lunisolar::month_basis::solar_term_month
            );

            assert(fp_midnight);
            assert(fp_zi);

            assert(fp_midnight.value.day != fp_zi.value.day);
        }

        {
            const sys_seconds utc{
                    sys_days{year{2024} / February / day{9}} +
                    hours{15} +
                    minutes{40}
            };

            const auto fp = Calendar::simplified_four_pillars(
                    utc,
                    seconds{8 * 3600 + 21 * 60},
                    lunisolar::day_boundary::zi_hour,
                    lunisolar::month_basis::solar_term_month
            );

            assert(fp);

            const auto parts = lunisolar::local_parts(
                    utc,
                    seconds{8 * 3600 + 21 * 60}
            );

            assert((parts.date == lunisolar::gregorian_date{2024, 2, 10}));
            assert(parts.hour == 0);
            assert(parts.minute == 1);
        }
    }

    void test_local_time_helpers() {
        using namespace std::chrono;

        const sys_seconds utc{
                sys_days{year{2024} / February / day{9}} + hours{16}
        };

        const auto local = lunisolar::local_time(utc, hours{8});

        const auto local_date = lunisolar::local_date(local);
        const auto local_hour = lunisolar::local_hour(local);

        assert((local_date == lunisolar::gregorian_date{2024, 2, 10}));
        assert(local_hour == 0);
    }

    void test_tuple_protocol() {
        {
            using type = lunisolar::chinese_date;

            static_assert(std::tuple_size_v<type> == 4);
            static_assert(std::is_same_v<std::tuple_element_t<0, type>, int>);
            static_assert(std::is_same_v<std::tuple_element_t<1, type>, unsigned>);
            static_assert(std::is_same_v<std::tuple_element_t<2, type>, unsigned>);
            static_assert(std::is_same_v<std::tuple_element_t<3, type>, bool>);

            type value{2024, 1, 1, false};

            assert(lunisolar::get<0>(value) == 2024);
            assert(lunisolar::get<1>(value) == 1);
            assert(lunisolar::get<2>(value) == 1);
            assert(lunisolar::get<3>(value) == false);

            auto [year, month, day, leap] = value;
            assert(year == value.year);
            assert(month == value.month);
            assert(day == value.day);
            assert(leap == value.leap);

            lunisolar::get<0>(value) = 2025;
            lunisolar::get<1>(value) = 2;
            lunisolar::get<2>(value) = 3;
            lunisolar::get<3>(value) = true;

            assert((value == type{2025, 2, 3, true}));
        }

        {
            using type = lunisolar::gregorian_date;

            static_assert(std::tuple_size_v<type> == 3);
            static_assert(std::is_same_v<std::tuple_element_t<0, type>, int>);
            static_assert(std::is_same_v<std::tuple_element_t<1, type>, unsigned>);
            static_assert(std::is_same_v<std::tuple_element_t<2, type>, unsigned>);

            type value{2024, 2, 10};

            assert(lunisolar::get<0>(value) == 2024);
            assert(lunisolar::get<1>(value) == 2);
            assert(lunisolar::get<2>(value) == 10);

            auto [year, month, day] = value;
            assert(year == value.year);
            assert(month == value.month);
            assert(day == value.day);

            lunisolar::get<0>(value) = 2025;
            lunisolar::get<1>(value) = 3;
            lunisolar::get<2>(value) = 4;

            assert((value == type{2025, 3, 4}));
        }

        {
            using type = lunisolar::local_date_time_parts;

            static_assert(std::tuple_size_v<type> == 4);
            static_assert(std::is_same_v<
                    std::tuple_element_t<0, type>,
                    lunisolar::gregorian_date
            >);
            static_assert(std::is_same_v<std::tuple_element_t<1, type>, unsigned>);
            static_assert(std::is_same_v<std::tuple_element_t<2, type>, unsigned>);
            static_assert(std::is_same_v<std::tuple_element_t<3, type>, unsigned>);

            type value{{2024, 2, 10}, 23, 21, 0};

            assert((lunisolar::get<0>(value) == lunisolar::gregorian_date{2024, 2, 10}));
            assert(lunisolar::get<1>(value) == 23);
            assert(lunisolar::get<2>(value) == 21);
            assert(lunisolar::get<3>(value) == 0);

            auto [date, hour, minute, second] = value;
            assert(date == value.date);
            assert(hour == value.hour);
            assert(minute == value.minute);
            assert(second == value.second);

            lunisolar::get<0>(value) = lunisolar::gregorian_date{2025, 3, 4};
            lunisolar::get<1>(value) = 1;
            lunisolar::get<2>(value) = 2;
            lunisolar::get<3>(value) = 3;

            assert((value == type{{2025, 3, 4}, 1, 2, 3}));
        }

        {
            using type = lunisolar::four_pillars;

            static_assert(std::tuple_size_v<type> == 4);
            static_assert(std::is_same_v<std::tuple_element_t<0, type>, lunisolar::ganzhi>);
            static_assert(std::is_same_v<std::tuple_element_t<1, type>, lunisolar::ganzhi>);
            static_assert(std::is_same_v<std::tuple_element_t<2, type>, lunisolar::ganzhi>);
            static_assert(std::is_same_v<std::tuple_element_t<3, type>, lunisolar::ganzhi>);

            type value{
                    lunisolar::ganzhi{0},
                    lunisolar::ganzhi{1},
                    lunisolar::ganzhi{2},
                    lunisolar::ganzhi{3}
            };

            assert(lunisolar::get<0>(value) == lunisolar::ganzhi{0});
            assert(lunisolar::get<1>(value) == lunisolar::ganzhi{1});
            assert(lunisolar::get<2>(value) == lunisolar::ganzhi{2});
            assert(lunisolar::get<3>(value) == lunisolar::ganzhi{3});

            auto [year, month, day, hour] = value;
            assert(year == value.year);
            assert(month == value.month);
            assert(day == value.day);
            assert(hour == value.hour);

            lunisolar::get<0>(value) = lunisolar::ganzhi{10};
            lunisolar::get<1>(value) = lunisolar::ganzhi{11};
            lunisolar::get<2>(value) = lunisolar::ganzhi{12};
            lunisolar::get<3>(value) = lunisolar::ganzhi{13};

            assert((value == type{
                    lunisolar::ganzhi{10},
                    lunisolar::ganzhi{11},
                    lunisolar::ganzhi{12},
                    lunisolar::ganzhi{13}
            }));
        }

        {
            using type = lunisolar::four_pillars_text;

            static_assert(std::tuple_size_v<type> == 4);
            static_assert(std::is_same_v<std::tuple_element_t<0, type>, std::string>);
            static_assert(std::is_same_v<std::tuple_element_t<1, type>, std::string>);
            static_assert(std::is_same_v<std::tuple_element_t<2, type>, std::string>);
            static_assert(std::is_same_v<std::tuple_element_t<3, type>, std::string>);

            type value{"甲辰", "丙寅", "甲子", "丙子"};

            assert(lunisolar::get<0>(value) == "甲辰");
            assert(lunisolar::get<1>(value) == "丙寅");
            assert(lunisolar::get<2>(value) == "甲子");
            assert(lunisolar::get<3>(value) == "丙子");

            auto [year, month, day, hour] = value;
            assert(year == value.year);
            assert(month == value.month);
            assert(day == value.day);
            assert(hour == value.hour);

            lunisolar::get<0>(value) = "year";
            lunisolar::get<1>(value) = "month";
            lunisolar::get<2>(value) = "day";
            lunisolar::get<3>(value) = "hour";

            assert(value.year == "year");
            assert(value.month == "month");
            assert(value.day == "day");
            assert(value.hour == "hour");
        }

        {
            using type = lunisolar::solar_term_year_month;

            static_assert(std::tuple_size_v<type> == 2);
            static_assert(std::is_same_v<std::tuple_element_t<0, type>, int>);
            static_assert(std::is_same_v<std::tuple_element_t<1, type>, unsigned>);

            type value{2024, 1};

            assert(lunisolar::get<0>(value) == 2024);
            assert(lunisolar::get<1>(value) == 1);

            auto [year, month] = value;
            assert(year == value.year);
            assert(month == value.month);

            lunisolar::get<0>(value) = 2025;
            lunisolar::get<1>(value) = 12;

            assert((value == type{2025, 12}));
        }
    }

} // namespace

int main() {
    test_packed_data_layout();
    test_calendar_matches_packed_data();
    test_full_round_trip_every_supported_day();
    test_boundaries();
    test_invalid_inputs();
    test_known_dates();

    test_solar_term_offsets();
    test_known_solar_terms();
    test_solar_term_year_month();

    test_ganzhi();
    test_text_mappings();
    test_solar_term_text_mappings();

    test_hour_ganzhi_with_offset();
    test_simplified_four_pillars();
    test_local_time_helpers();

    test_tuple_protocol();

    std::cout << "lunisolar tests passed\n";
}
