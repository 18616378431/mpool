#ifndef DURATION_H_
#define	DURATION_H_

#include <chrono>

using Microseconds = std::chrono::microseconds;

using Milliseconds = std::chrono::milliseconds;

using Seconds = std::chrono::seconds;

using Minutes = std::chrono::minutes;

using Hours = std::chrono::hours;

#if defined(__GNUC__) && (!defined(__clang__) || (__clang_major__ == 10))
using Days = std::chrono::duration<__INT64_TYPE__, std::ratio<86400>>;

using Weeks = std::chrono::duration<__INT64_TYPE__, std::ratio<604800>>;

using Years = std::chrono::duration<__INT64_TYPE__, std::ratio<31556952>>;

using Months = std::chrono::duration<__INT64_TYPE__, std::ratio<2629746>>;

#else
using Days = std::chrono::days;

using Weeks = std::chrono::weeks;

using Years = std::chrono::years;

using Months = std::chrono::months;

#endif

using TimePoint = std::chrono::steady_clock::time_point;
using SystemTimePoint = std::chrono::system_clock::time_point;

using namespace std::chrono_literals;

constexpr Days operator""_days(unsigned long long days)
{
	return Days(days);
}

constexpr Weeks operator""_weeks(unsigned long long weeks)
{
	return Weeks(weeks);
}

constexpr Years operator""_years(unsigned long long years)
{
	return Years(years);
}

constexpr Months operator""_months(unsigned long long months)
{
	return Months(months);
}

#endif