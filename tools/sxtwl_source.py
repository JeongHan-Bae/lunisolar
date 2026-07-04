from __future__ import annotations

"""sxtwl-backed calendar data source implementation."""

from dataclasses import dataclass
from datetime import date, timedelta
from typing import Any

try:
    import sxtwl
except ModuleNotFoundError as exc:
    raise ModuleNotFoundError(
        "The 'sxtwl' package is required to generate lunisolar data. "
        "Create the project virtual environment and install dependencies with:\n"
        "python3 -m venv .venv\n"
        "source .venv/bin/activate\n"
        "python -m pip install --upgrade pip\n"
        "python -m pip install sxtwl"
    ) from exc

from calendar_source import CalendarSource, LunarDay

SOLAR_TERM_ORDER: list[str] = [
    "立春", "雨水", "惊蛰", "春分", "清明", "谷雨",
    "立夏", "小满", "芒种", "夏至", "小暑", "大暑",
    "立秋", "处暑", "白露", "秋分", "寒露", "霜降",
    "立冬", "小雪", "大雪", "冬至", "小寒", "大寒",
]

SXTWL_FALLBACK_JQMC: list[str] = [
    "冬至", "小寒", "大寒", "立春", "雨水", "惊蛰",
    "春分", "清明", "谷雨", "立夏", "小满", "芒种",
    "夏至", "小暑", "大暑", "立秋", "处暑", "白露",
    "秋分", "寒露", "霜降", "立冬", "小雪", "大雪",
]

KNOWN_DAY_GANZHI_ANCHOR_DATE = date(2019, 1, 27)
KNOWN_DAY_GANZHI_ANCHOR_INDEX = 0


def floor_mod(value: int, modulus: int) -> int:
    """Return a non-negative modulo result."""

    result = value % modulus
    return result + modulus if result < 0 else result


def day_has_jieqi(day: Any) -> bool:
    """Return whether an ``sxtwl`` day exposes a solar term."""

    if hasattr(day, "hasJieQi"):
        return bool(day.hasJieQi())
    if hasattr(day, "hasJieqi"):
        return bool(day.hasJieqi())
    if hasattr(day, "hasJQ"):
        return bool(day.hasJQ())
    raise RuntimeError("sxtwl day object does not expose hasJieQi/hasJieqi/hasJQ")


def day_jieqi_index(day: Any) -> int:
    """Return the solar-term index for an ``sxtwl`` day."""

    if hasattr(day, "getJieQi"):
        return int(day.getJieQi())
    if hasattr(day, "getJieqi"):
        return int(day.getJieqi())
    if hasattr(day, "getJQ"):
        return int(day.getJQ())
    raise RuntimeError("sxtwl day object does not expose getJieQi/getJieqi/getJQ")


def sxtwl_jieqi_names() -> list[str]:
    """Return the solar-term names exposed by ``sxtwl``."""

    names = getattr(sxtwl, "Jqmc", None)
    if names is None:
        names = getattr(sxtwl, "jqmc", None)
    if names is None:
        return SXTWL_FALLBACK_JQMC

    result = [str(x) for x in names]
    if len(result) < 24:
        return SXTWL_FALLBACK_JQMC
    return result


@dataclass(frozen=True)
class SxtwlSource(CalendarSource):
    """Calendar source backed by the ``sxtwl`` package."""

    def day_ganzhi_index_for_date(self, value: date) -> int:
        """Return the day Ganzhi index for a Gregorian date."""

        delta = (value - KNOWN_DAY_GANZHI_ANCHOR_DATE).days
        return floor_mod(delta + KNOWN_DAY_GANZHI_ANCHOR_INDEX, 60)

    def lunar_from_solar(self, value: date) -> LunarDay:
        """Convert a Gregorian date to a lunisolar date using ``sxtwl``."""

        x = sxtwl.fromSolar(value.year, value.month, value.day)
        return LunarDay(
            year=int(x.getLunarYear(True)),
            month=int(x.getLunarMonth()),
            day=int(x.getLunarDay()),
            leap=bool(x.isLunarLeap()),
        )

    def solar_term_dates_for_year(self, year: int) -> list[date]:
        """Collect all 24 solar-term dates for one Gregorian year."""

        found: dict[str, date] = {}
        names = sxtwl_jieqi_names()

        current = date(year, 1, 1)
        end = date(year, 12, 31)

        while current <= end:
            day = sxtwl.fromSolar(current.year, current.month, current.day)
            if day_has_jieqi(day):
                index = day_jieqi_index(day)
                if index < 0 or index >= len(names):
                    raise RuntimeError(f"{current}: JieQi index out of range: {index}")

                name = names[index]
                if name in SOLAR_TERM_ORDER:
                    if name in found:
                        raise RuntimeError(f"{year}: duplicate solar term {name}")
                    found[name] = current

            current += timedelta(days=1)

        missing = [name for name in SOLAR_TERM_ORDER if name not in found]
        if missing:
            raise RuntimeError(
                f"{year}: missing solar terms {missing}; "
                f"detected={sorted(found.items(), key=lambda x: x[1])}; "
                f"sxtwl_names={names}"
            )

        return [found[name] for name in SOLAR_TERM_ORDER]
