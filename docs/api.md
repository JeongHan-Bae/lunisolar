# lunisolar API Reference

This document describes the public API exposed by `lunisolar.h` in terms of the
existing examples, tests, and implementation details in this repository.

## Header and dataset includes

```cpp
#include "lunisolar.h"
#include "lunisolar_data.h" // generated packed dataset
```

All examples and tests use:

```cpp
using Calendar = lunisolar::data::calendar;
```

The alias is a concrete `lunisolar::basic_calendar` instantiated with the generated
dataset constants.

## Version metadata

```cpp
namespace lunisolar::meta {
    inline constexpr int major;
    inline constexpr int minor;
    inline constexpr int patch;
    inline const std::string_view version;
}
```

Current values are set in `lunisolar.h` and formatted by `meta_version_storage()`.

## Global helpers

### Supported-year utility

```cpp
template<int FirstSupportedYear, int LastSupportedYear>
requires (LastSupportedYear >= FirstSupportedYear)
consteval std::size_t supported_year_count();
```

Returns the number of years in a closed range.

### Constants

```cpp
inline constexpr std::size_t solar_term_count = 24;
inline constexpr std::size_t packed_solar_term_word_count = 2;
```

### Error handling

```cpp
enum class error {
    none,
    year_out_of_range,
    invalid_gregorian_date,
    invalid_chinese_month,
    invalid_chinese_day,
    invalid_leap_month
};

template<class T>
struct result {
    T value{};
    error ec{error::none};
    constexpr explicit operator bool() const noexcept;
};
```

All checked APIs in the public surface return `result<T>` and are `noexcept`.

## Data types

### Date structs

```cpp
struct chinese_date { int year; unsigned month; unsigned day; bool leap; };
struct gregorian_date { int year; unsigned month; unsigned day; };
struct local_date_time_parts { gregorian_date date; unsigned hour; unsigned minute; unsigned second; };
```

All provide `operator==`.

### Sexagenary and stems/branches

```cpp
enum class heavenly_stem : std::uint8_t { jia, yi, bing, ding, wu, ji, geng, xin, ren, gui };
enum class earthly_branch : std::uint8_t { zi, chou, yin, mao, chen, si, wu, wei, shen, you, xu, hai };

struct ganzhi {
    std::uint8_t index{};
    constexpr ganzhi() noexcept = default;
    constexpr explicit ganzhi(unsigned value) noexcept; // modulo 60
    constexpr heavenly_stem stem() const noexcept;
    constexpr earthly_branch branch() const noexcept;
};

struct four_pillars { ganzhi year, month, day, hour; };
struct four_pillars_text { std::string year, month, day, hour; };
```

### Solar-term metadata

```cpp
enum class text_style { hans_cn, hant_cn, pinyin, eon_mun_ko_cn, numeric };
enum class day_boundary { midnight, zi_hour };
enum class month_basis { lunar_month, solar_term_month };

enum class solar_term : std::uint8_t {
    lichun, yushui, jingzhe, chunfen, qingming, guyu,
    lixia, xiaoman, mangzhong, xiazhi, xiaoshu, dashu,
    liqiu, chushu, bailu, qiufen, hanlu, shuangjiang,
    lidong, xiaoxue, daxue, dongzhi, xiaohan, dahan
};

struct solar_term_year_month { int year; unsigned month; };
struct solar_term_date_info { solar_term term; gregorian_date date; };
```

## Text conversion

```cpp
std::string to_string(heavenly_stem value, text_style style = text_style::hans_cn);
std::string to_string(earthly_branch value, text_style style = text_style::hans_cn);
std::string to_string(solar_term value, text_style style = text_style::hans_cn);
std::string to_string(ganzhi value, text_style style = text_style::hans_cn);
four_pillars_text to_string(four_pillars value, text_style style = text_style::hans_cn);
std::string join(four_pillars value,
                 text_style style = text_style::hans_cn,
                 std::string_view separator = " ");
```

`text_style::numeric` returns index-style strings for the same values.

`text_style` options:

- `hans_cn`: Simplified character form.
- `hant_cn`: Traditional character set.
- `pinyin`: Hanyu Pinyin lower-case Latin.
- `eon_mun_ko_cn`: Joseonjok Eon-Mun (Hangul).
- `numeric`: Zero-based decimal index string (`0`..`59` for `ganzhi`, etc.).

`to_string` enumerations:

- `heavenly_stem`: `jia`, `yi`, `bing`, `ding`, `wu`, `ji`, `geng`, `xin`, `ren`, `gui`.
- `earthly_branch`: `zi`, `chou`, `yin`, `mao`, `chen`, `si`, `wu`, `wei`, `shen`, `you`, `xu`, `hai`.
- `solar_term`: `lichun`, `yushui`, `jingzhe`, `chunfen`, `qingming`, `guyu`,
  `lixia`, `xiaoman`, `mangzhong`, `xiazhi`, `xiaoshu`, `dashu`,
  `liqiu`, `chushu`, `bailu`, `qiufen`, `hanlu`, `shuangjiang`,
  `lidong`, `xiaoxue`, `daxue`, `dongzhi`, `xiaohan`, `dahan`.
- `ganzhi`: combines a stem (`index % 10`) and branch (`index % 12`) into one value (`0` is Jia-Zi, `59` is Gui-Hai); pinyin output inserts `-` between components.

## Time helper overloads

```cpp
template<class Duration>
constexpr auto local_time(std::chrono::sys_time<Duration> utc, std::chrono::seconds utc_offset) noexcept;

template<class Duration>
constexpr gregorian_date local_date(std::chrono::local_time<Duration> value) noexcept;

template<class Duration>
constexpr unsigned local_hour(std::chrono::local_time<Duration> value) noexcept;

template<class Duration>
constexpr local_date_time_parts local_parts(std::chrono::local_time<Duration> value) noexcept;

template<class Duration>
constexpr local_date_time_parts local_parts(std::chrono::sys_time<Duration> utc, std::chrono::seconds utc_offset) noexcept;
```

## Calendar adapter

### `basic_calendar` template

```cpp
template<
    int FirstSupportedYear,
    int LastSupportedYear,
    gregorian_date DayGanzhiAnchorDate,
    unsigned DayGanzhiAnchorIndex,
    const std::array<std::uint64_t, supported_year_count<...>()>& PackedYears,
    const std::array<std::array<std::uint64_t, packed_solar_term_word_count>,
                   supported_year_count<...>()>& PackedSolarTerms
>
class basic_calendar { ... };
```

`lunisolar::data::calendar` is a ready-made alias to the generated dataset.

#### Static properties

```cpp
static constexpr int first_supported_year;
static constexpr int last_supported_year;
static constexpr gregorian_date day_ganzhi_anchor_date;
static constexpr unsigned day_ganzhi_anchor_index;
using days = std::chrono::days;
using sys_days = std::chrono::sys_days;
```

#### API surface

#### Support and range checks

```cpp
static constexpr std::size_t supported_year_count() noexcept;
```
Return the supported year span size computed from template bounds.

```cpp
static constexpr bool supports_chinese_year(int year) noexcept;
```
Check whether a lunar year is present in the packed dataset.

```cpp
static constexpr bool supports_gregorian_year(int year) noexcept;
```
Check whether a Gregorian year is within supported boundaries.

```cpp
static constexpr bool supports_gregorian_date(gregorian_date date) noexcept;
```
Check whether a Gregorian date is valid and within supported boundaries.

```cpp
static constexpr gregorian_date first_gregorian_date() noexcept;
```
Return the earliest supported Gregorian date.

```cpp
static constexpr gregorian_date last_gregorian_date() noexcept;
```
Return the latest supported Gregorian date.

#### Year and month metadata

```cpp
static constexpr std::uint64_t packed_year(int year) noexcept;
```
Return packed metadata for a year; returns `0` when the year is unsupported.

```cpp
static constexpr unsigned month_bits(int year) noexcept;
```
Return the month-length bit field for a year; returns `0` when unsupported.

```cpp
static constexpr unsigned leap_month(int year) noexcept;
```
Return the leap month number in the range `1..12`, or `0` if no leap month exists.

```cpp
static constexpr unsigned new_year_day(int year) noexcept;
```
Return the Lunar New Year day-of-year offset of the supported Gregorian year.

```cpp
static constexpr unsigned year_length(int year) noexcept;
```
Return the total day count of a lunar year.

```cpp
static constexpr unsigned physical_month_count(int year) noexcept;
```
Return the number of physical months in the year (`12` or `13`).

```cpp
static constexpr unsigned physical_month_days(int year, unsigned physical_index) noexcept;
```
Return days in a physical month by one-based index.

```cpp
static constexpr unsigned regular_month_days(int year, unsigned month, bool is_leap = false) noexcept;
```
Return days for logical month `month`; set `is_leap` to read leap month layout.

```cpp
static constexpr sys_days chinese_new_year(int year) noexcept;
```
Return the Gregorian `std::chrono::sys_days` for Lunar New Year.

```cpp
static constexpr bool valid(chinese_date value) noexcept;
```
Validate a lunar date object against field and leap constraints.

```cpp
static constexpr result<gregorian_date> to_gregorian(chinese_date value) noexcept;
```
Convert lunar date to Gregorian date. Returns an error on invalid input or unsupported year.

```cpp
static constexpr result<chinese_date> from_gregorian(gregorian_date value) noexcept;
```
Convert Gregorian date to lunar date. Returns an error on invalid input or unsupported year.

#### Ganzhi and pillar calculations

```cpp
static constexpr ganzhi year_ganzhi(int chinese_year) noexcept;
```
Compute the year pillar for a lunar year.

```cpp
static constexpr ganzhi month_ganzhi(int ganzhi_year, unsigned month) noexcept;
```
Compute the month pillar from lunar year and month number.

```cpp
static constexpr ganzhi day_ganzhi(gregorian_date date) noexcept;
```
Compute the day pillar from a Gregorian date.

```cpp
static constexpr ganzhi hour_ganzhi(
    gregorian_date date,
    unsigned hour,
    day_boundary boundary = day_boundary::midnight
) noexcept;
```
Compute the hour pillar from local date and hour with boundary rule.

```cpp
template<class Duration>
static constexpr ganzhi hour_ganzhi(
    std::chrono::sys_time<Duration> utc,
    std::chrono::seconds utc_offset,
    day_boundary boundary = day_boundary::midnight
) noexcept;
```
Compute the hour pillar from UTC time and fixed offset.

```cpp
template<class Duration>
static constexpr result<ganzhi> checked_hour_ganzhi(
    std::chrono::sys_time<Duration> utc,
    std::chrono::seconds utc_offset,
    day_boundary boundary = day_boundary::midnight
) noexcept;
```
Checked version of UTC hour conversion, returning `result<ganzhi>`.

#### Solar-term and four-pillars utilities

```cpp
static constexpr result<gregorian_date> solar_term_date(int gregorian_year, solar_term term) noexcept;
```
Return Gregorian date on which a specific solar term occurs.

```cpp
static constexpr result<solar_term_date_info> previous_solar_term(gregorian_date date) noexcept;
```
Return the nearest prior solar term record.

```cpp
static constexpr result<solar_term_date_info> next_solar_term(gregorian_date date) noexcept;
```
Return the nearest following solar term record.

```cpp
static constexpr result<solar_term_date_info> nearest_solar_term(gregorian_date date) noexcept;
```
Return the closer of previous and next terms, and choose previous when equidistant.

```cpp
static constexpr result<solar_term_year_month> solar_term_year_month_of(gregorian_date date) noexcept;
```
Return the Li-Chun-based month-year pairing for the date.

```cpp
static constexpr result<ganzhi> solar_term_month_ganzhi(gregorian_date date) noexcept;
```
Compute the month pillar based on solar-term month position.

```cpp
static constexpr result<four_pillars> simplified_four_pillars(
    gregorian_date date,
    unsigned hour,
    day_boundary boundary = day_boundary::midnight,
    month_basis basis = month_basis::solar_term_month
) noexcept;
```
Compute four pillars from local date and hour. `basis` controls lunar vs solar-term month mode.

```cpp
template<class Duration>
static constexpr result<four_pillars> simplified_four_pillars(
    std::chrono::sys_time<Duration> utc,
    std::chrono::seconds utc_offset,
    day_boundary boundary = day_boundary::midnight,
    month_basis basis = month_basis::solar_term_month
) noexcept;
```
Compute four pillars from UTC plus fixed offset.

### Behavior notes from tests/examples

- `from_gregorian()` and `to_gregorian()` must succeed only on dates in the exact
  supported range (`1715-02-04` through `2227-02-16` with the default dataset).
- Leap-month conversions fail with `error::invalid_leap_month` if source date
  refers to a leap month that does not exist in that year.
- `solar_term_year_month_of()` uses Li-Chun to determine the solar-term-based year.
- `nearest_solar_term()` returns the earlier term when equidistant from previous and next.
- `checked_hour_ganzhi()` validates date conversion through `from_gregorian()` and returns
  the same error code as the conversion failure when out-of-range.

## Tuple/structured-binding protocol

`get<I>(obj)` overloads and `std::tuple_size` / `std::tuple_element` specializations
are provided for:

- `chinese_date`, `gregorian_date`, `local_date_time_parts`,
  `four_pillars`, `four_pillars_text`, `solar_term_year_month`, `solar_term_date_info`.

This allows:

```cpp
auto [year, month, day, leap] = lunar_date;
```

and assignment back through `lunisolar::get<>()`.

## Quick usage patterns

### Conversion and formatting

```cpp
using Calendar = lunisolar::data::calendar;

const auto lunar = Calendar::from_gregorian({2026, 2, 17});
if (lunar) {
    std::cout << lunar.value.year << '-' << lunar.value.month << '-' << lunar.value.day
              << (lunar.value.leap ? " (leap)" : "") << '\n';
}

const auto greg = Calendar::to_gregorian({2026, 1, 1, false});
if (greg) {
    const auto str = lunisolar::to_string(
        Calendar::year_ganzhi(greg.value.year),
        lunisolar::text_style::pinyin
    );
}
```

### Four Pillars with local time

```cpp
using namespace std::chrono;
const sys_seconds utc{sys_days{year{2024}/February/day{9}} + hours{15}};
const auto pillars = Calendar::simplified_four_pillars(utc, hours{8});
if (pillars) {
    std::cout << lunisolar::join(pillars.value) << '\n';
}
```

### Calendar and solar-term lookups

```cpp
const auto lichun = Calendar::solar_term_date(2026, lunisolar::solar_term::lichun);
const auto prev = Calendar::previous_solar_term({2026, 2, 10});
const auto nearest = Calendar::nearest_solar_term({2026, 2, 10});
const auto next = Calendar::next_solar_term({2026, 2, 10});
```
