from __future__ import annotations

"""Abstract calendar source interfaces for data generation."""

from abc import ABC, abstractmethod
from dataclasses import dataclass
from datetime import date


@dataclass(frozen=True)
class LunarDay:
    """One Chinese lunisolar calendar date."""

    year: int
    month: int
    day: int
    leap: bool


class CalendarSource(ABC):
    """Abstract source of lunisolar calendar and solar-term data."""

    @abstractmethod
    def day_ganzhi_index_for_date(self, value: date) -> int:
        """Return the zero-based day Ganzhi index for a Gregorian date."""

    @abstractmethod
    def lunar_from_solar(self, value: date) -> LunarDay:
        """Convert a Gregorian date to a lunisolar date."""

    @abstractmethod
    def solar_term_dates_for_year(self, year: int) -> list[date]:
        """Return all 24 solar-term dates for a Gregorian year."""
