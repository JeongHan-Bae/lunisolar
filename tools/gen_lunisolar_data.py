#!/usr/bin/env python3
from __future__ import annotations

"""Generate packed lunisolar data artifacts from pluggable calendar sources.

This script extracts Chinese lunisolar dates and solar terms from a calendar
source implementation, packs them into the compact layout consumed by the C++
header-only library, validates the generated tables, and writes the generated
artifacts used by the project. Its primary outputs are the includable
``lunisolar_data.h`` header and test-side ``*.inc`` data snippets.
"""

import argparse
import importlib
from dataclasses import dataclass
from datetime import date, timedelta
from pathlib import Path
from typing import Iterable

from calendar_source import CalendarSource, LunarDay

DEFAULT_FIRST_YEAR = 1715
DEFAULT_LAST_YEAR = 2226
DEFAULT_SOURCE_MODULE = "sxtwl_source"
DEFAULT_SOURCE_CLASS = "SxtwlSource"

SOLAR_TERM_ORDER: list[str] = [
    "立春", "雨水", "惊蛰", "春分", "清明", "谷雨",
    "立夏", "小满", "芒种", "夏至", "小暑", "大暑",
    "立秋", "处暑", "白露", "秋分", "寒露", "霜降",
    "立冬", "小雪", "大雪", "冬至", "小寒", "大寒",
]

SOLAR_TERM_BASE: list[tuple[int, int]] = [
    (2, 4), (2, 19), (3, 6), (3, 21), (4, 5), (4, 20),
    (5, 6), (5, 21), (6, 6), (6, 21), (7, 7), (7, 23),
    (8, 8), (8, 23), (9, 8), (9, 23), (10, 8), (10, 23),
    (11, 7), (11, 22), (12, 7), (12, 22), (1, 6), (1, 20),
]

SOLAR_TERM_OFFSET_BIAS = 8
SOLAR_TERM_OFFSET_MIN = -8
SOLAR_TERM_OFFSET_MAX = 7


@dataclass(frozen=True)
class MonthStart:
    """Start of one physical lunisolar month."""

    solar: date
    lunar_month: int
    leap: bool


@dataclass(frozen=True)
class PackedYear:
    """Packed representation of one supported Chinese year."""

    year: int
    month_bits: int
    leap_month: int
    new_year_day: int
    year_length: int
    packed: int


@dataclass(frozen=True)
class PackedSolarTerms:
    """Packed solar-term offsets and decoded dates for one Gregorian year."""

    year: int
    words: tuple[int, int]
    dates: tuple[date, ...]


@dataclass(frozen=True)
class DayGanzhiAnchor:
    """Reference day used to reconstruct day Ganzhi values."""

    date: date
    index: int


def supported_year_count(first_year: int, last_year: int) -> int:
    """Return the inclusive number of supported years.

    Args:
        first_year: First supported Chinese year.
        last_year: Last supported Chinese year.

    Returns:
        The number of years in the closed interval.
    """

    return last_year - first_year + 1


def validate_year_range(first_year: int, last_year: int) -> None:
    """Validate an inclusive year range.

    Args:
        first_year: First supported Chinese year.
        last_year: Last supported Chinese year.

    Raises:
        ValueError: If ``first_year`` is greater than ``last_year``.
    """

    if first_year > last_year:
        raise ValueError(
            f"invalid year range: first_year={first_year}, last_year={last_year}"
        )


def find_chinese_new_year(source: CalendarSource, year: int) -> date:
    """Find Chinese New Year for a supported Gregorian year.

    Args:
        source: Calendar data source.
        year: Gregorian year whose Chinese New Year should be found.

    Returns:
        Gregorian date of Chinese New Year.

    Raises:
        RuntimeError: If no matching first day of the first lunar month is found.
    """

    begin = date(year, 1, 1)
    end = date(year, 3, 15)

    d = begin
    while d <= end:
        ld = source.lunar_from_solar(d)
        if ld.year == year and ld.month == 1 and ld.day == 1 and not ld.leap:
            return d
        d += timedelta(days=1)

    raise RuntimeError(f"cannot find Chinese New Year for {year}")


def collect_month_starts(
        source: CalendarSource,
        year: int,
) -> tuple[date, date, list[MonthStart]]:
    """Collect every physical month start in one Chinese year.

    Args:
        source: Calendar data source.
        year: Chinese year to inspect.

    Returns:
        A tuple of the current Chinese New Year, the next Chinese New Year, and
        all physical month starts in between.

    Raises:
        RuntimeError: If a scanned day resolves to an unexpected lunar year.
    """

    cny = find_chinese_new_year(source, year)
    next_cny = find_chinese_new_year(source, year + 1)

    starts: list[MonthStart] = []
    d = cny
    while d < next_cny:
        ld = source.lunar_from_solar(d)
        if ld.year != year:
            raise RuntimeError(f"{d} belongs to lunar year {ld.year}, expected {year}")

        if ld.day == 1:
            starts.append(MonthStart(d, ld.month, ld.leap))

        d += timedelta(days=1)

    return cny, next_cny, starts


def pack_year(source: CalendarSource, year: int) -> PackedYear:
    """Pack one Chinese year into the compact C++ storage layout.

    Args:
        source: Calendar data source.
        year: Chinese year to pack.

    Returns:
        Packed representation for the year.

    Raises:
        RuntimeError: If the extracted calendar structure is invalid.
    """

    cny, next_cny, starts = collect_month_starts(source, year)

    if len(starts) not in (12, 13):
        raise RuntimeError(f"{year}: invalid physical month count {len(starts)}")

    regular_months = [x.lunar_month for x in starts if not x.leap]
    if regular_months != list(range(1, 13)):
        raise RuntimeError(f"{year}: invalid regular month sequence {regular_months}")

    leap_starts = [x for x in starts if x.leap]
    if len(leap_starts) > 1:
        raise RuntimeError(f"{year}: more than one leap month")

    leap_month = leap_starts[0].lunar_month if leap_starts else 0

    if leap_month:
        regular_index = next(
            i for i, x in enumerate(starts)
            if x.lunar_month == leap_month and not x.leap
        )
        leap_index = next(
            i for i, x in enumerate(starts)
            if x.lunar_month == leap_month and x.leap
        )
        if leap_index != regular_index + 1:
            raise RuntimeError(f"{year}: leap month is not after regular month")

    all_starts = starts + [MonthStart(next_cny, 1, False)]

    month_bits = 0
    month_days: list[int] = []

    for i in range(len(starts)):
        days = (all_starts[i + 1].solar - all_starts[i].solar).days
        if days not in (29, 30):
            raise RuntimeError(f"{year}: invalid month length {days}")

        month_days.append(days)
        if days == 30:
            month_bits |= 1 << i

    year_length = sum(month_days)
    if year_length != (next_cny - cny).days:
        raise RuntimeError(f"{year}: year length mismatch")

    if not (353 <= year_length <= 385):
        raise RuntimeError(f"{year}: suspicious year length {year_length}")

    new_year_day = (cny - date(year, 1, 1)).days
    if not (0 <= new_year_day <= 60):
        raise RuntimeError(f"{year}: suspicious new_year_day {new_year_day}")

    packed = (
            (month_bits & 0x1FFF)
            | ((leap_month & 0xF) << 13)
            | ((new_year_day & 0x1FF) << 17)
            | ((year_length & 0x1FF) << 26)
    )

    return PackedYear(
        year=year,
        month_bits=month_bits,
        leap_month=leap_month,
        new_year_day=new_year_day,
        year_length=year_length,
        packed=packed,
    )


def decode_packed_year(value: int) -> tuple[int, int, int, int]:
    """Decode one packed Chinese year.

    Args:
        value: Packed year bit field.

    Returns:
        Tuple of ``(month_bits, leap_month, new_year_day, year_length)``.
    """

    month_bits = value & 0x1FFF
    leap_month = (value >> 13) & 0xF
    new_year_day = (value >> 17) & 0x1FF
    year_length = (value >> 26) & 0x1FF
    return month_bits, leap_month, new_year_day, year_length


def load_calendar_source(module_name: str, class_name: str) -> CalendarSource:
    """Load and instantiate a calendar source implementation dynamically."""

    module = importlib.import_module(module_name)

    try:
        source_class = getattr(module, class_name)
    except AttributeError as exc:
        raise RuntimeError(
            f"calendar source class {class_name!r} not found in module {module_name!r}"
        ) from exc

    source = source_class()

    if not isinstance(source, CalendarSource):
        raise TypeError(f"{module_name}.{class_name} must implement CalendarSource")

    return source


def physical_month_count(leap_month: int) -> int:
    """Return the physical month count for a year.

    Args:
        leap_month: Leap month number, or ``0`` when absent.

    Returns:
        ``13`` for leap years, otherwise ``12``.
    """

    return 13 if leap_month else 12


def physical_month_days(month_bits: int, physical_index: int) -> int:
    """Return the length of one physical month.

    Args:
        month_bits: Packed month-length bit field.
        physical_index: Zero-based physical month index.

    Returns:
        ``30`` for a big month, otherwise ``29``.
    """

    return 30 if ((month_bits >> physical_index) & 1) else 29


def logical_month_from_physical(leap_month: int, physical: int) -> tuple[int, bool]:
    """Map a physical month index to a logical month and leap flag.

    Args:
        leap_month: Leap month number, or ``0`` when absent.
        physical: Zero-based physical month index.

    Returns:
        Tuple of ``(logical_month, is_leap)``.
    """

    if leap_month == 0 or physical < leap_month:
        return physical + 1, False
    if physical == leap_month:
        return leap_month, True
    return physical, False


def first_supported_gregorian_date(lunar_table: list[PackedYear]) -> date:
    """Return the first Gregorian date covered by the packed table.

    Args:
        lunar_table: Packed lunar years in ascending year order.

    Returns:
        First supported Gregorian date.

    Raises:
        RuntimeError: If the table is empty.
    """

    if not lunar_table:
        raise RuntimeError("empty lunar table")
    row = lunar_table[0]
    return date(row.year, 1, 1) + timedelta(days=row.new_year_day)


def last_supported_gregorian_date(lunar_table: list[PackedYear]) -> date:
    """Return the last Gregorian date covered by the packed table.

    Args:
        lunar_table: Packed lunar years in ascending year order.

    Returns:
        Last supported Gregorian date.

    Raises:
        RuntimeError: If the table is empty.
    """

    if not lunar_table:
        raise RuntimeError("empty lunar table")
    row = lunar_table[-1]
    cny = date(row.year, 1, 1) + timedelta(days=row.new_year_day)
    return cny + timedelta(days=row.year_length - 1)


def choose_day_ganzhi_anchor(
        source: CalendarSource,
        lunar_table: list[PackedYear],
) -> DayGanzhiAnchor:
    """Choose a stable day Ganzhi anchor near the supported midpoint.

    Args:
        source: Calendar data source.
        lunar_table: Packed lunar years in ascending year order.

    Returns:
        Anchor date and zero-based Ganzhi index.
    """

    first = first_supported_gregorian_date(lunar_table)
    last = last_supported_gregorian_date(lunar_table)

    midpoint = first + timedelta(days=(last - first).days // 2)
    return DayGanzhiAnchor(
        date=midpoint,
        index=source.day_ganzhi_index_for_date(midpoint),
    )


def validate_day_ganzhi_anchor(
        source: CalendarSource,
        anchor: DayGanzhiAnchor,
) -> None:
    """Validate a day Ganzhi anchor against the known reference rule.

    Args:
        source: Calendar data source.
        anchor: Anchor to validate.

    Raises:
        RuntimeError: If the stored Ganzhi index is inconsistent.
    """

    expected = source.day_ganzhi_index_for_date(anchor.date)
    if anchor.index != expected:
        raise RuntimeError(
            f"day ganzhi anchor mismatch: date={anchor.date}, "
            f"index={anchor.index}, expected={expected}"
        )


def validate_all_lunar_days(
        source: CalendarSource,
        table: list[PackedYear],
        first_year: int,
        last_year: int,
) -> None:
    """Cross-check packed lunar data against the active calendar source day by day.

    Args:
        source: Calendar data source.
        table: Packed lunar year rows.
        first_year: First supported Chinese year.
        last_year: Last supported Chinese year.

    Raises:
        RuntimeError: If any reconstructed lunar date differs from the source.
    """

    packed_by_year = {x.year: x for x in table}

    first = find_chinese_new_year(source, first_year)
    final_exclusive = find_chinese_new_year(source, last_year + 1)

    d = first
    while d < final_exclusive:
        actual = source.lunar_from_solar(d)

        cy = d.year
        if cy > last_year:
            cy = last_year
        else:
            candidate = packed_by_year.get(cy)
            if candidate is None:
                cy -= 1
            else:
                if (d - date(d.year, 1, 1)).days < candidate.new_year_day:
                    cy -= 1

        if cy < first_year or cy > last_year:
            raise RuntimeError(f"{d}: calculated year out of range {cy}")

        row = packed_by_year[cy]
        rem = (d - (date(cy, 1, 1) + timedelta(days=row.new_year_day))).days

        month_bits, leap_month, _, year_length = decode_packed_year(row.packed)
        if rem < 0 or rem >= year_length:
            raise RuntimeError(f"{d}: invalid remaining day {rem}")

        physical = 0
        for physical in range(physical_month_count(leap_month)):
            mdays = physical_month_days(month_bits, physical)
            if rem < mdays:
                break
            rem -= mdays

        month, is_leap = logical_month_from_physical(leap_month, physical)
        calculated = LunarDay(cy, month, rem + 1, is_leap)

        if calculated != actual:
            raise RuntimeError(
                f"{d}: mismatch, calculated={calculated}, actual={actual}"
            )

        d += timedelta(days=1)


def pack_solar_term_offsets(year: int, term_dates: list[date]) -> PackedSolarTerms:
    """Pack solar-term offsets for one Gregorian year.

    Args:
        year: Gregorian year to pack.
        term_dates: Solar-term dates in ``SOLAR_TERM_ORDER``.

    Returns:
        Packed solar-term row.

    Raises:
        RuntimeError: If term count or encoded offsets are invalid.
    """

    if len(term_dates) != 24:
        raise RuntimeError("expected 24 solar terms")

    words = [0, 0]

    for i, actual in enumerate(term_dates):
        month, day = SOLAR_TERM_BASE[i]
        base = date(year, month, day)
        offset = (actual - base).days

        if not (SOLAR_TERM_OFFSET_MIN <= offset <= SOLAR_TERM_OFFSET_MAX):
            raise RuntimeError(
                f"{year} {SOLAR_TERM_ORDER[i]}: offset {offset} out of range, "
                f"actual={actual}, base={base}"
            )

        encoded = offset + SOLAR_TERM_OFFSET_BIAS
        if not (0 <= encoded <= 0xF):
            raise RuntimeError(f"{year} {SOLAR_TERM_ORDER[i]}: invalid encoded offset")

        bit = i * 4
        word = bit // 64
        shift = bit % 64

        words[word] |= encoded << shift

    return PackedSolarTerms(
        year=year,
        words=(words[0], words[1]),
        dates=tuple(term_dates),
    )


def unpack_solar_term_offset(words: tuple[int, int], term_index: int) -> int:
    """Unpack one solar-term offset from two packed words.

    Args:
        words: Two packed words for one Gregorian year.
        term_index: Zero-based solar-term index.

    Returns:
        Signed day offset from the stable base date.
    """

    bit = term_index * 4
    word = bit // 64
    shift = bit % 64
    raw = (words[word] >> shift) & 0xF
    return raw - SOLAR_TERM_OFFSET_BIAS


def validate_solar_terms(table: list[PackedSolarTerms]) -> None:
    """Validate packed solar-term rows against stored absolute dates.

    Args:
        table: Packed solar-term rows.

    Raises:
        RuntimeError: If unpacking does not reproduce the stored dates.
    """

    for row in table:
        for i, expected in enumerate(row.dates):
            month, day = SOLAR_TERM_BASE[i]
            base = date(row.year, month, day)
            actual = base + timedelta(days=unpack_solar_term_offset(row.words, i))
            if actual != expected:
                raise RuntimeError(
                    f"{row.year} {SOLAR_TERM_ORDER[i]}: "
                    f"unpack mismatch actual={actual}, expected={expected}"
                )


def pack_all_solar_terms(
        source: CalendarSource,
        first_year: int,
        last_year: int,
) -> list[PackedSolarTerms]:
    """Pack solar terms for all supported Gregorian years.

    Args:
        source: Calendar data source.
        first_year: First supported Gregorian year.
        last_year: Last supported Gregorian year.

    Returns:
        Packed solar-term rows for the inclusive range.
    """

    rows: list[PackedSolarTerms] = []
    for year in range(first_year, last_year + 1):
        dates = source.solar_term_dates_for_year(year)
        rows.append(pack_solar_term_offsets(year, dates))
    validate_solar_terms(rows)
    return rows


def iter_test_vector_lines(
        source: CalendarSource,
        first_year: int,
        last_year: int,
) -> Iterable[str]:
    """Yield test-vector rows for month starts and lunar year endings.

    Args:
        source: Calendar data source.
        first_year: First supported Chinese year.
        last_year: Last supported Chinese year.

    Yields:
        One formatted initializer line per test vector.
    """

    for year in range(first_year, last_year + 1):
        _, next_cny, starts = collect_month_starts(source, year)
        for start in starts:
            yield (
                f"    {{{start.solar.year}, {start.solar.month}, {start.solar.day}, "
                f"{year}, {start.lunar_month}, 1, {'true' if start.leap else 'false'}}},"
            )

        last_day = next_cny - timedelta(days=1)
        last_lunar = source.lunar_from_solar(last_day)
        yield (
            f"    {{{last_day.year}, {last_day.month}, {last_day.day}, "
            f"{last_lunar.year}, {last_lunar.month}, {last_lunar.day}, "
            f"{'true' if last_lunar.leap else 'false'}}},"
        )


def emit_test_vectors(
        source: CalendarSource,
        first_year: int,
        last_year: int,
) -> tuple[str, str]:
    """Render test-vector include contents.

    Args:
        source: Calendar data source.
        first_year: First supported Chinese year.
        last_year: Last supported Chinese year.

    Returns:
        Tuple of ``(vectors_text, count_text)``.
    """

    lines = list(iter_test_vector_lines(source, first_year, last_year))
    return "\n".join(lines) + "\n", f"{len(lines)}\n"


def emit_cpp_header(
        lunar_table: list[PackedYear],
        solar_table: list[PackedSolarTerms],
        first_year: int,
        last_year: int,
        day_ganzhi_anchor: DayGanzhiAnchor,
) -> str:
    """Render the generated dataset as a C++ header.

    Args:
        lunar_table: Packed lunar year rows.
        solar_table: Packed solar-term rows.
        first_year: First supported Chinese year.
        last_year: Last supported Chinese year.
        day_ganzhi_anchor: Chosen day Ganzhi anchor.

    Returns:
        Full C++ header text.

    Raises:
        RuntimeError: If the generated row counts do not match the year range.
    """

    year_count = supported_year_count(first_year, last_year)

    if len(lunar_table) != year_count:
        raise RuntimeError("invalid lunar table length")
    if len(solar_table) != year_count:
        raise RuntimeError("invalid solar term table length")

    first_gregorian = first_supported_gregorian_date(lunar_table)
    last_gregorian = last_supported_gregorian_date(lunar_table)

    lines: list[str] = []
    lines.append("/**")
    lines.append(" * @file lunisolar_data.h")
    lines.append(" * @brief Generated packed lunisolar dataset for the lunisolar library.")
    lines.append(" *")
    lines.append(" * @details")
    lines.append(" * ")
    lines.append(" * This file was automatically generated by")
    lines.append(" * \"tools/gen_lunisolar_data.py\" for the lunisolar library.")
    lines.append(" * Do not edit it manually.")
    lines.append(" * <br>")
    lines.append(" * The generator script and the generated output are distributed under")
    lines.append(" * the MIT License as part of the lunisolar project.")
    lines.append(" */")
    lines.append("#pragma once")
    lines.append("")
    lines.append("#include <array>")
    lines.append("#include <cstdint>")
    lines.append("")
    lines.append("#include \"lunisolar.h\"")
    lines.append("")
    lines.append("namespace lunisolar::data {")
    lines.append("")
    lines.append(f"inline constexpr int first_supported_year = {first_year};")
    lines.append(f"inline constexpr int last_supported_year = {last_year};")
    lines.append("")
    lines.append("inline constexpr lunisolar::gregorian_date day_ganzhi_anchor_date{")
    lines.append(f"    {day_ganzhi_anchor.date.year},")
    lines.append(f"    {day_ganzhi_anchor.date.month},")
    lines.append(f"    {day_ganzhi_anchor.date.day}")
    lines.append("};")
    lines.append("")
    lines.append(
        f"inline constexpr unsigned day_ganzhi_anchor_index = "
        f"{day_ganzhi_anchor.index}U;"
    )
    lines.append("")
    lines.append(
        f"// Supported Gregorian date range: "
        f"{first_gregorian.isoformat()} through {last_gregorian.isoformat()}."
    )
    lines.append(
        f"// Day ganzhi anchor is selected near the middle of this range."
    )
    lines.append("")
    lines.append(
        "inline constexpr std::array<"
        "std::uint64_t, "
        "lunisolar::supported_year_count<first_supported_year, last_supported_year>()"
        "> packed_years{"
    )
    for row in lunar_table:
        lines.append(
            f"    0x{row.packed:016X}ULL, "
            f"// {row.year}: month_bits=0x{row.month_bits:04X}, "
            f"leap={row.leap_month}, new_year_day={row.new_year_day}, "
            f"year_length={row.year_length}"
        )
    lines.append("};")
    lines.append("")
    lines.append(
        "inline constexpr std::array<"
        "std::array<std::uint64_t, lunisolar::packed_solar_term_word_count>, "
        "lunisolar::supported_year_count<first_supported_year, last_supported_year>()"
        "> packed_solar_terms{"
    )
    for row in solar_table:
        comment_terms = ", ".join(
            f"{SOLAR_TERM_ORDER[i]}={row.dates[i].month:02d}-{row.dates[i].day:02d}"
            for i in range(24)
        )
        lines.append(
            f"    std::array<std::uint64_t, lunisolar::packed_solar_term_word_count>{{"
            f"0x{row.words[0]:016X}ULL, "
            f"0x{row.words[1]:016X}ULL"
            f"}}, // {row.year}: {comment_terms}"
        )
    lines.append("};")
    lines.append("")
    lines.append("using calendar = lunisolar::calendar<")
    lines.append("        first_supported_year,")
    lines.append("        last_supported_year,")
    lines.append("        day_ganzhi_anchor_date,")
    lines.append("        day_ganzhi_anchor_index,")
    lines.append("        packed_years,")
    lines.append("        packed_solar_terms")
    lines.append(">;")
    lines.append("")
    lines.append("} // namespace lunisolar::data")
    lines.append("")
    return "\n".join(lines)


def main() -> None:
    """Run the command-line generator."""

    parser = argparse.ArgumentParser()
    parser.add_argument("--out", required=True, help="output C++ header path")
    parser.add_argument(
        "--first-year",
        type=int,
        default=DEFAULT_FIRST_YEAR,
        help=f"first supported Chinese year, default: {DEFAULT_FIRST_YEAR}",
    )
    parser.add_argument(
        "--last-year",
        type=int,
        default=DEFAULT_LAST_YEAR,
        help=f"last supported Chinese year, default: {DEFAULT_LAST_YEAR}",
    )
    parser.add_argument(
        "--source-module",
        default=DEFAULT_SOURCE_MODULE,
        help=f"calendar source module, default: {DEFAULT_SOURCE_MODULE}",
    )
    parser.add_argument(
        "--source-class",
        default=DEFAULT_SOURCE_CLASS,
        help=f"calendar source class, default: {DEFAULT_SOURCE_CLASS}",
    )
    test_vector_group = parser.add_mutually_exclusive_group()
    test_vector_group.add_argument(
        "--test-vectors",
        required=False,
        help="output path for generated test vectors include",
    )
    test_vector_group.add_argument(
        "--no-test-vectors",
        action="store_true",
        help="do not generate test vectors",
    )
    args = parser.parse_args()

    first_year = args.first_year
    last_year = args.last_year
    validate_year_range(first_year, last_year)

    source = load_calendar_source(args.source_module, args.source_class)

    lunar_table = [
        pack_year(source, year) for year in range(first_year, last_year + 1)
    ]
    validate_all_lunar_days(source, lunar_table, first_year, last_year)

    solar_table = pack_all_solar_terms(source, first_year, last_year)

    day_ganzhi_anchor = choose_day_ganzhi_anchor(source, lunar_table)
    validate_day_ganzhi_anchor(source, day_ganzhi_anchor)

    out = Path(args.out)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(
        emit_cpp_header(
            lunar_table=lunar_table,
            solar_table=solar_table,
            first_year=first_year,
            last_year=last_year,
            day_ganzhi_anchor=day_ganzhi_anchor,
        ),
        encoding="utf-8",
    )

    if args.test_vectors and not args.no_test_vectors:
        vectors_text, count_text = emit_test_vectors(source, first_year, last_year)
        vectors_path = Path(args.test_vectors)
        vectors_path.parent.mkdir(parents=True, exist_ok=True)
        vectors_path.write_text(vectors_text, encoding="utf-8")

        count_path = vectors_path.with_name(
            vectors_path.name.replace(".inc", "_count.inc")
        )
        count_path.write_text(count_text, encoding="utf-8")

    print(f"generated {out}")
    if args.test_vectors and not args.no_test_vectors:
        print(f"generated {args.test_vectors}")
    print(f"supported Chinese years: {first_year}..{last_year}")
    print(
        "supported Gregorian dates: "
        f"{first_supported_gregorian_date(lunar_table)}.."
        f"{last_supported_gregorian_date(lunar_table)}"
    )
    print(
        "day ganzhi anchor: "
        f"{day_ganzhi_anchor.date} index={day_ganzhi_anchor.index}"
    )
    print("lunar validation passed")
    print("solar term validation passed")


if __name__ == "__main__":
    main()
