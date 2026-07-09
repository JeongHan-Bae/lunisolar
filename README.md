<h1 align="center">
  <span>
    <img src="https://upload.wikimedia.org/wikipedia/commons/1/18/ISO_C%2B%2B_Logo.svg" 
         alt="C++ Logo" 
         width="64" valign="middle">
  </span>
  <span style="font-size: x-large;">&nbsp;lunisolar</span>
</h1>

<p align="center">
  <img
    src="https://img.shields.io/endpoint?url=https://raw.githubusercontent.com/JeongHan-Bae/lunisolar/main/version_badge.json"
    alt="badge"
    width="196"
  >
</p>

<p align="center">
  <a href="https://github.com/JeongHan-Bae/lunisolar/actions/workflows/ci.yaml?query=event%3Apull_request">
    <img
      src="https://github.com/JeongHan-Bae/lunisolar/actions/workflows/ci.yaml/badge.svg?event=pull_request"
      alt="CI"
      width="196"
    >
  </a>
</p>

<p align="center">
  <a href="https://zread.ai/JeongHan-Bae/lunisolar">
    <img
      src="https://img.shields.io/badge/Ask_Zread-_.svg?style=flat&color=00b0aa&labelColor=323232&logo=data%3Aimage%2Fsvg%2Bxml%3Bbase64%2CPHN2ZyB3aWR0aD0iMTYiIGhlaWdodD0iMTYiIHZpZXdCb3g9IjAgMCAxNiAxNiIgZmlsbD0ibm9uZSIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj4KPHBhdGggZD0iTTQuOTYxNTYgMS42MDAxSDIuMjQxNTZDMS44ODgxIDEuNjAwMSAxLjYwMTU2IDEuODg2NjQgMS42MDE1NiAyLjI0MDFWNC45NjAxQzEuNjAxNTYgNS4zMTM1NiAxLjg4ODEgNS42MDAxIDIuMjQxNTYgNS42MDAxSDQuOTYxNTZDNS4zMTUwMiA1LjYwMDEgNS42MDE1NiA1LjMxMzU2IDUuNjAxNTYgNC45NjAxVjIuMjQwMUM1LjYwMTU2IDEuODg2NjQgNS4zMTUwMiAxLjYwMDEgNC45NjE1NiAxLjYwMDFaIiBmaWxsPSIjZmZmIi8%2BCjxwYXRoIGQ9Ik00Ljk2MTU2IDEwLjM5OTlIMi4yNDE1NkMxLjg4ODEgMTAuMzk5OSAxLjYwMTU2IDEwLjY4NjQgMS42MDE1NiAxMS4wMzk5VjEzLjc1OTlDMS42MDE1NiAxNC4xMTM0IDEuODg4MSAxNC4zOTk5IDIuMjQxNTYgMTQuMzk5OUg0Ljk2MTU2QzUuMzE1MDIgMTQuMzk5OSA1LjYwMTU2IDE0LjExMzQgNS42MDE1NiAxMy43NTk5VjExLjAzOTlDNS42MDE1NiAxMC42ODY0IDUuMzE1MDIgMTAuMzk5OSA0Ljk2MTU2IDEwLjM5OTlaIiBmaWxsPSIjZmZmIi8%2BCjxwYXRoIGQ9Ik0xMy43NTg0IDEuNjAwMUgxMS4wMzg0QzEwLjY4NSAxLjYwMDEgMTAuMzk4NCAxLjg4NjY0IDEwLjM5ODQgMi4yNDAxVjQuOTYwMUMxMC4zOTg0IDUuMzEzNTYgMTAuNjg1IDUuNjAwMSAxMS4wMzg0IDUuNjAwMUgxMy43NTg0QzE0LjExMTkgNS42MDAxIDE0LjM5ODQgNS4zMTM1NiAxNC4zOTg0IDQuOTYwMVYyLjI0MDFDMTQuMzk4NCAxLjg4NjY0IDE0LjExMTkgMS42MDAxIDEzLjc1ODQgMS42MDAxWiIgZmlsbD0iI2ZmZiIvPgo8cGF0aCBkPSJNNCAxMkwxMiA0TDQgMTJaIiBmaWxsPSIjZmZmIi8%2BCjxwYXRoIGQ9Ik00IDEyTDEyIDQiIHN0cm9rZT0iI2ZmZiIgc3Ryb2tlLXdpZHRoPSIxLjUiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIvPgo8L3N2Zz4K&logoColor=ffffff"
      alt="Ask Zread"
      width="104"
    >
  </a>
</p>

<p align="center">
  <img
    src="https://img.shields.io/github/languages/top/JeongHan-Bae/lunisolar?style=flat&label=C%2B%2B&labelColor=323232&color=e85050"
    alt="Top Language"
    width="96"
  >
</p>

<font size="5"><code>lunisolar</code></font> is a header-only C++20 library for converting between Gregorian dates and the Chinese lunisolar calendar.
It is a backend-agnostic, fully precomputed calendar encoding system: the runtime library only decodes generated packed
data, and `sxtwl` is just one optional generator backend for producing that dataset rather than a runtime dependency or
the source of the library's core logic.

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

## Data Model

The C++ library is intentionally data-driven. Its runtime behavior comes from packed `lunisolar_data` tables generated
ahead of time and compiled into the final binary. The calendar conversion logic in the header is therefore independent of
any particular upstream library as long as the dataset is prepared in the expected format.

This separation is deliberate:

- runtime conversion depends only on precomputed packed data plus the C++ decoding logic
- dataset generation can use any backend that can provide the required lunisolar dates, solar terms, and anchors
- `sxtwl` is bundled only as one ready-made backend implementation for producing the repository's default dataset

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

#include <iostream>

using Calendar = lunisolar::data::calendar;

int main() {
    const auto result = Calendar::from_gregorian({2026, 2, 17});
    if (!result) {
        std::cerr << "Failed to convert Gregorian date.\n";
        return 1;
    }

    const auto chinese = result.value;
    std::cout << "Chinese date: "
              << chinese.year
              << '-'
              << chinese.month
              << '-'
              << chinese.day
              << (chinese.leap ? " (leap)\n" : "\n");

    const auto solar_term_result =
        Calendar::solar_term_date(2026, lunisolar::solar_term::lichun);
    if (!solar_term_result) {
        std::cerr << "Failed to resolve Li-Chun.\n";
        return 1;
    }

    const auto lichun = solar_term_result.value;
    std::cout << "Li-Chun: "
              << lichun.year
              << '-'
              << lichun.month
              << '-'
              << lichun.day
              << '\n';

    return 0;
}
```

## Repository Layout

```text
lunisolar/
├── .github/
│   └── workflows/
│       ├── ci.yaml             # GitHub Actions CI workflow
│       └── update-version.yaml # workflow for updating version badge metadata
├── docs/
│   └── api.md                  # public API reference
├── examples/
│   ├── birthdays.cpp           # example: map one Chinese birthday across many Chinese years
│   └── solar_terms.cpp         # example: sample moon-phase-style output and nearby solar terms
├── include/
│   └── lunisolar.h             # main header-only library interface
├── scripts/
│   └── gen_version_badge.py    # generate version_badge.json from header metadata
├── tests/
│   └── test_lunisolar.cpp      # tests proving packed data and runtime decoding stay faithful to the source data
├── tools/
│   ├── calendar_source.py      # calendar source interface required by the generator
│   ├── gen_lunisolar_data.py   # core backend-agnostic data packing and validation script
│   ├── gen_lunisolar_data.sh   # shell wrapper for data generation
│   └── sxtwl_source.py         # optional CalendarSource implementation backed by sxtwl
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

## Generator Backends

The Python generation pipeline is designed around a pluggable source interface rather than a hard dependency on `sxtwl`.
`tools/calendar_source.py` defines the interface expected by the generator, and the core script
`tools/gen_lunisolar_data.py` handles the packing, validation, and artifact emission once a source object satisfies that
interface.

In other words, the core script's job is data compression and validation. If a provided class can supply:

- Gregorian-to-lunisolar conversion
- solar-term dates
- the day Ganzhi anchor information used by the generated dataset

then the script can compress that source data into the packed format consumed by the C++ library. The bundled
`tools/sxtwl_source.py` is only one implementation of that contract.

## Tests

The tests are intended to show two things:

- the packed dataset generated by the Python tooling preserves the source calendar information faithfully
- the C++ runtime decoding and conversion APIs behave consistently with that packed dataset

This means the test suite is not just checking surface examples. It is also validating that the compression pipeline and
the library's calculations remain faithful to the underlying source data used to generate the tables.

## License

This project is licensed under the MIT License. See [LICENSE](https://github.com/JeongHan-Bae/lunisolar?tab=MIT-1-ov-file).
