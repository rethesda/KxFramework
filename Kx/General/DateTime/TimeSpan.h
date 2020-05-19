#pragma once
#include "Common.h"

namespace KxFramework
{
	class Locale;
	class ITimeClock;
}

namespace KxFramework::Private::DateTime
{
	constexpr int MillisecondsInSecond = 1000;
	constexpr int MillisecondsInMinute = MillisecondsInSecond * 60;
	constexpr int MillisecondsInHour = MillisecondsInMinute * 60;
	constexpr int MillisecondsInDay = MillisecondsInHour * 24;
	constexpr int MillisecondsInWeek = MillisecondsInDay * 7;
}

namespace KxFramework
{
	class KX_API TimeSpan final
	{
		public:
			static constexpr TimeSpan Milliseconds(int64_t milliseconds) noexcept
			{
				return milliseconds;
			}
			static constexpr TimeSpan Seconds(int64_t seconds) noexcept
			{
				return seconds * Private::DateTime::MillisecondsInSecond;
			}
			static constexpr TimeSpan Minutes(int64_t minutes) noexcept
			{
				return minutes * Private::DateTime::MillisecondsInMinute;
			}
			static constexpr TimeSpan Hours(int64_t hours) noexcept
			{
				return hours * Private::DateTime::MillisecondsInHour;
			}
			static constexpr TimeSpan Days(int64_t days) noexcept
			{
				return days * Private::DateTime::MillisecondsInDay;
			}
			static constexpr TimeSpan Weeks(int64_t weeks) noexcept
			{
				return weeks * Private::DateTime::MillisecondsInWeek;
			}

			static TimeSpan Now() noexcept;
			static TimeSpan Now(const ITimeClock& clock) noexcept;

		private:
			int64_t m_Value = 0;

		private:
			constexpr TimeSpan(int64_t value) noexcept
				:m_Value(value)
			{
			}

		public:
			constexpr TimeSpan() noexcept = default;
			constexpr TimeSpan(const TimeSpan&) noexcept = default;
			TimeSpan(const wxTimeSpan& other) noexcept
				:m_Value(other.GetMilliseconds().GetValue())
			{
			}

		public:
			constexpr bool IsNull() const noexcept
			{
				return m_Value == 0;
			}
			constexpr bool IsNegative() const noexcept
			{
				return m_Value < 0;
			}
			constexpr bool IsPositive() const noexcept
			{
				return m_Value > 0;
			}

			constexpr int64_t GetWeeks() const noexcept
			{
				return m_Value / Private::DateTime::MillisecondsInWeek;
			}
			constexpr int64_t GetDays() const noexcept
			{
				return m_Value / Private::DateTime::MillisecondsInDay;
			}
			constexpr int64_t GetHours() const noexcept
			{
				return m_Value / Private::DateTime::MillisecondsInHour;
			}
			constexpr int64_t GetMinutes() const noexcept
			{
				return m_Value / Private::DateTime::MillisecondsInMinute;
			}
			constexpr int64_t GetSeconds() const noexcept
			{
				return m_Value / Private::DateTime::MillisecondsInSecond;
			}
			constexpr int64_t GetMilliseconds() const noexcept
			{
				return m_Value;
			}

			constexpr TimeSpan Abs() const noexcept
			{
				return TimeSpan(m_Value < 0 ? -m_Value : m_Value);
			}
			constexpr TimeSpan Negate() const noexcept
			{
				return TimeSpan(-m_Value);
			}

			// Compare two time spans. Works with the absolute values.
			constexpr bool IsLongerThan(const TimeSpan& other) const noexcept
			{
				return Abs() > other.Abs();
			}
			constexpr bool IsShorterThan(const TimeSpan& other) const noexcept
			{
				return Abs() < other.Abs();
			}

			String Format(const String& format) const;
			String Format(const char* format) const
			{
				return Format(String(format));
			}
			String Format(const wchar_t* format) const
			{
				return Format(String(format));
			}
			String Format(const Locale& locale, TimeFormatFlag flags = TimeFormatFlag::None) const;

		public:
			constexpr TimeSpan& operator=(const TimeSpan&) noexcept = default;
			TimeSpan& operator=(const wxTimeSpan& other) noexcept
			{
				m_Value = other.GetMilliseconds().GetValue();
				return *this;
			}
			operator wxTimeSpan() const noexcept
			{
				return TimeSpan::GetMilliseconds();
			}

			constexpr bool operator==(const TimeSpan& other) const noexcept
			{
				return m_Value == other.m_Value;
			}
			constexpr bool operator!=(const TimeSpan& other) const noexcept
			{
				return !(*this == other);
			}
			constexpr bool operator<(const TimeSpan& other) const noexcept
			{
				return m_Value < other.m_Value;
			}
			constexpr bool operator<=(const TimeSpan& other) const noexcept
			{
				return m_Value <= other.m_Value;
			}
			constexpr bool operator>(const TimeSpan& other) const noexcept
			{
				return m_Value > other.m_Value;
			}
			constexpr bool operator>=(const TimeSpan& other) const noexcept
			{
				return m_Value >= other.m_Value;
			}

			constexpr TimeSpan& operator+=(const TimeSpan& other) noexcept
			{
				m_Value += other.m_Value;
				return *this;
			}
			constexpr TimeSpan& operator-=(const TimeSpan& other) noexcept
			{
				m_Value -= other.m_Value;
				return *this;
			}
			constexpr TimeSpan& operator*=(int64_t multiplier) noexcept
			{
				m_Value *= multiplier;
				return *this;
			}

			constexpr TimeSpan operator-() const noexcept
			{
				return Negate();
			}
			constexpr TimeSpan operator+(const TimeSpan& other) const noexcept
			{
				return m_Value + other.m_Value;
			}
			constexpr TimeSpan operator-(const TimeSpan& other) const noexcept
			{
				return m_Value - other.m_Value;
			}
			constexpr TimeSpan operator*(int64_t multiplier) const noexcept
			{
				return m_Value * multiplier;
			}
	};
}
