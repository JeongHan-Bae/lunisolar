# lunisolar

[![CI](https://github.com/JeongHan-Bae/lunisolar/actions/workflows/ci.yaml/badge.svg?event=pull_request)](https://github.com/JeongHan-Bae/lunisolar/actions/workflows/ci.yaml?query=event%3Apull_request)

`lunisolar` is a header-only C++20 library for converting between Gregorian dates and the Chinese lunisolar calendar.

### Scope

Here, `lunisolar` specifically means the Chinese lunisolar calendar, that is, the traditional Chinese calendar commonly
called the Chinese lunar calendar (`nongli`). It does not mean a purely lunar calendar in the literal sense, such as the
Islamic calendar.

### Why "Lunisolar"

For readers unfamiliar with the Latin roots, `lunisolar` literally means a calendar defined by both the moon and the
sun: `luna` is the Latin root behind `moon`-related words, and `sol`/`solar` is the Latin root behind `sun`-related
words. In practical terms, a lunisolar month is aligned with the lunar phase cycle, so a month is treated as a complete
lunar month rather than an arbitrary solar subdivision. At the same time, the calendar also tracks solar terms, so it
remains legible against the sun-based seasonal year.

As a result, a lunisolar year is not just a count of lunar months. Instead, it is organized so that lunar months are not
cut apart, while the year as a whole stays close to the tropical year through intercalation. In broad terms, one may
think of it as a calendar where each month fully covers a lunar-phase cycle, and each year oscillates around the solar
year while preserving whole lunar months.

### Standardization Target

This library models the Chinese calendrical system standardized in Mainland China. In that sense, the Chinese lunisolar
calendar and the traditional Chinese calendar it represents are treated here as part of traditional Chinese culture,
implemented according to the modern Mainland Chinese standard `GB/T 33661-2017`.

The author is only interested in the Gregorian calendar side of the mapping. Accordingly, this library only maps between
Gregorian dates and the Chinese lunisolar calendar.

### Supported Date Range

This library does not support 1582 or earlier, does not support the Julian calendar or other older calendrical systems,
and does not attempt historical back-calculation across pre-modern calendar regimes. It is not intended for historical
calendrical reconstruction.

In practice, it should be used for modern and near-modern date reasoning only. The implementation is intentionally
fitted to modern standardized dates and to a Gregorian year model that behaves like an ordinary approximately 365-day
solar year. This constraint keeps the packed lookup tables small enough to remain practical as hardcoded compiler-stored
data.

The generated dataset included in this repository defaults to the inclusive Chinese-year range `1715..2226`. These are
Chinese lunisolar years, not a bare Gregorian-year clamp. In Gregorian terms, the covered date span is `1715-02-04`
through `2227-02-16`, that is, from the first day of the first month of Chinese year `1715` through the last day of the
last month of Chinese year `2226`.

### Features

It uses generated packed data tables and provides:

- Gregorian ↔ Chinese lunisolar date conversion
- Chinese New Year, leap month, and month-length lookup
- Ganzhi helpers for year, month, day, and hour
- Local-time helpers for fixed-offset UTC conversion, including `local_time`, `local_date`, `local_hour`, and hour-based
  Ganzhi / Four Pillars lookup
- Solar term lookup
- Text output for stems, branches, Ganzhi, and solar terms in Mainland simplified Chinese, Mainland traditional Chinese,
  Hanyu Pinyin, and Joseonjok EonMun (Hangul)

The library follows the Mainland Chinese lunisolar standard `GB/T 33661-2017` and does not provide locale-specific
calendar variants.

### Time And Hour Handling

For hour-based calculations, the library supports both direct local civil input such as `(gregorian_date, hour)` and UTC
input plus a fixed `utc_offset`. The helper `local_hour(...)` extracts the local clock hour in the range `0..23`, which
is then used by APIs such as hour Ganzhi and simplified Four Pillars.

### Output Styles

The output surface intentionally provides only four textual styles and one numeric style. The four textual styles are
not four separate languages. They are two Hanzi writing forms and two pronunciation-based spellings for the same Hanzi
standard:

- Mainland simplified Chinese
- Mainland traditional Chinese
- Hanyu Pinyin
- Joseonjok EonMun (Hangul)

The single numeric style is provided as a neutral index-style representation for code paths that want stable
machine-facing identifiers instead of orthographic forms.

This restriction is deliberate. Except for numeric output, the library only exposes one-to-one mappings that stay
anchored to the Mainland Chinese calendrical standard `GB/T 33661-2017` and the corresponding Mainland Chinese Hanzi
standard. The textual surface is limited to two Mainland Chinese Hanzi writing styles and two Hanzi-based pronunciation
spellings. No further language, dialect, or locale expansion is provided.

### Non-Goals

In particular, translation is out of scope. The library does not support translation into other language systems,
dialect writing systems, minority-language renderings, or historically reconstructed naming systems.

In short: no i18n.

## Checked APIs

Checked conversion and lookup APIs return `lunisolar::result<T>`.

### Error Model

All core calendrical conversion, lookup, validation, local-time extraction, and Four Pillars computation APIs are
declared `noexcept`.

The main exception is the text materialization layer such as `to_string(...)` and `join(...)`, which returns owning
`std::string` values and is therefore not presented as a blanket `noexcept` surface.

`result<T>` is a small C++20-friendly simplification of `std::expected<T, E>` from C++23:

- `result.value` stores the computed value on success
- `result.ec` stores the error code on failure
- `explicit operator bool()` returns whether the operation succeeded

In other words, `result<T>` is the library's unified way to return either an answer value or an error code while staying
within C++20.

## Example

```cpp
#include "lunisolar.h"
#include "lunisolar_data.h"

#include <cassert>

using Calendar = lunisolar::data::calendar;

int main() {
    const auto result = Calendar::from_gregorian({2026, 2, 17});
    assert(result); // equivalent to: assert(result.ec == lunisolar::error::none);

    const auto chinese = result.value;
    assert(chinese.year == 2026);
    assert(chinese.month == 1);
    assert(chinese.day == 1);
    assert(!chinese.leap); // true means this month instance is a leap month

    const auto solar_term_result =
        Calendar::solar_term_date(2026, lunisolar::solar_term::lichun);
    assert(solar_term_result);

    const auto lichun = solar_term_result.value;
    assert(lichun.year == 2026);
    assert(lichun.month == 2);
    assert(lichun.day == 4);
}
```

## Repository Layout

```text
lunisolar/
├── .github/
│   └── workflows/
│       └── ci.yaml           # GitHub Actions CI workflow
├── include/
│   └── lunisolar.h           # main header-only library interface
├── tests/
│   └── test_lunisolar.cpp    # unit tests
├── tools/
│   ├── calendar_source.py    # calendar data source generator interface
│   ├── gen_lunisolar_data.py # lunisolar data generator
│   ├── gen_lunisolar_data.sh # shell wrapper for data generation
│   └── sxtwl_source.py       # realization of calendar_source using sxtwl library
├── .gitignore
├── AGENTS.md
├── CMakeLists.txt
├── LICENSE
└── README.md
```

## Build

This repository uses CMake and requires C++20.

Generated data is produced by:

```bash
bash tools/gen_lunisolar_data.sh
```

The shell wrapper installs dependencies if needed and then runs `tools/gen_lunisolar_data.py` with the repository
defaults.

Compared with the shell wrapper, `tools/gen_lunisolar_data.py` additionally supports passing explicit generation
parameters such as the supported year range and output paths.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE).
