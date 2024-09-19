#ifndef TIMER_H_
#define TIMER_H_

#include "Common.h"
#include "Duration.h"

enum class TimeFormat : uint8
{
	FullText,//1 days 2hours 3 minutes 4 seconds 5 milliseconds
	ShortText,//1d 2h 3m 4s 5ms
	Numeric//1:2:3:4:5
};

enum class TimeOutput : uint8
{
	Days,
	Hours,
	Minutes,
	Seconds,
	Milliseconds,
	Microseconds
};

namespace mpool::Time
{
	template<typename T>
	POOL_COMMON_API uint32 TimeStringTo(std::string_view timeString);

	template<typename T>
	POOL_COMMON_API std::string ToTimeString(uint64 durationTime, TimeOutput timeOutput = TimeOutput::Seconds, TimeFormat timeFormat = TimeFormat::ShortText);
	
	template<typename T>
	POOL_COMMON_API std::string ToTimeString(std::string_view durationTime, TimeOutput timeOutput = TimeOutput::Seconds, TimeFormat timeFormat = TimeFormat::ShortText);

	POOL_COMMON_API std::string ToTimeString(Microseconds durationTime, TimeOutput timeOutput = TimeOutput::Seconds, TimeFormat timeFormat = TimeFormat::ShortText);

	POOL_COMMON_API time_t LocalTimeToUTCTime(time_t time);
	POOL_COMMON_API time_t GetLocalHourTimestamp(time_t time, uint8 hour, bool onlyAfterTime = true);
	POOL_COMMON_API std::tm TimeBreakdown(time_t t = 0);
	POOL_COMMON_API std::string TimeToTimestampStr(Seconds time = 0s, std::string_view fmt = {});
	POOL_COMMON_API std::string TimeToHumanReadable(Seconds time = 0s, std::string_view fmt = {});

	POOL_COMMON_API time_t GetNextTimeWithDayAndHour(int8 dayOfWeek, int8 hour);
	POOL_COMMON_API time_t GetNextTimeWithMonthAndHour(int8 month, int8 hour);

	POOL_COMMON_API uint32 GetSeconds(Seconds time = 0s);
	POOL_COMMON_API uint32 GetMinutes(Seconds time = 0s);
	POOL_COMMON_API uint32 GetHours(Seconds time = 0s);
	POOL_COMMON_API uint32 GetDayInWeek(Seconds time = 0s);
	POOL_COMMON_API uint32 GetDayInMonth(Seconds time = 0s);
	POOL_COMMON_API uint32 GetDayInYear(Seconds time = 0s);
	POOL_COMMON_API uint32 GetMonth(Seconds time = 0s);
	POOL_COMMON_API uint32 GetYear(Seconds time = 0s);
}

POOL_COMMON_API struct tm* localtime_r(time_t const* time, struct tm* result);

inline TimePoint GetApplicationStartTime()
{
	using namespace std::chrono;
	
	static const TimePoint ApplicationStartTime = steady_clock::now();

	return ApplicationStartTime;
}

inline Milliseconds GetTimeMS()
{
	using namespace std::chrono;

	return duration_cast<milliseconds>(steady_clock::now() - GetApplicationStartTime());
}

inline Milliseconds GetMSTimeDiff(Milliseconds oldMSTime, Milliseconds newMSTime)
{
	if (oldMSTime > newMSTime)
	{
		return oldMSTime - newMSTime;
	}
	else
	{
		return newMSTime - oldMSTime;
	}
}

inline uint32 getMSTime()
{
	using namespace std::chrono;

	return uint32(duration_cast<milliseconds>(steady_clock::now() - GetApplicationStartTime()).count());
}

inline uint32 getMSTimeDiff(uint32 oldMSTime, uint32 newMSTime)
{
	if (oldMSTime > newMSTime)
	{
		return (0xFFFFFFFF - oldMSTime) + newMSTime;
	}
	else
	{
		return newMSTime - oldMSTime;
	}
}

inline uint32 getMSTimeDiff(uint32 oldMSTime, TimePoint newTime)
{
	using namespace std::chrono;

	uint32 newMSTime = uint32(duration_cast<milliseconds>(newTime - GetApplicationStartTime()).count());

	return getMSTimeDiff(oldMSTime, newMSTime);
}

inline uint32 GetMSTimeDiffToNow(uint32 oldMSTime)
{
	return getMSTimeDiff(oldMSTime, getMSTime());
}

inline Milliseconds GetMSTimeDiffToNow(Milliseconds oldMSTime)
{
	return GetMSTimeDiff(oldMSTime, GetTimeMS());
}

inline Seconds GetEpochTime()
{
	using namespace std::chrono;

	return duration_cast<Seconds>(system_clock::now().time_since_epoch());
}

struct IntervalTimer
{
public:
	IntervalTimer() = default;

	void Update(time_t diff)
	{
		_current += diff;
		if (_current < 0)
		{
			_current = 0;
		}
	}

	bool Passed()
	{
		return _current >= _interval;
	}

	void Reset()
	{
		if (_current >= _interval)
		{
			_current %= _interval;
		}
	}

	void SetCurrent(time_t current)
	{
		_current = current;
	}

	void SetInterval(time_t interval)
	{
		_interval = interval;
	}

	[[nodiscard]] time_t GetInterval() const
	{
		return _interval;
	}

	[[nodiscard]] time_t GetCurrent() const
	{
		return _current;
	}

private:
	time_t _interval{0};
	time_t _current{0};
};

struct TimeTracker
{
public:
	TimeTracker(time_t expiry)
		: i_expiryTime(expiry)
	{
	}

	void Update(time_t diff)
	{
		i_expiryTime -= diff;
	}

	[[nodiscard]] bool Passed() const
	{
		return i_expiryTime <= 0;
	}

	void Reset(time_t interval)
	{
		i_expiryTime = interval;
	}

	[[nodiscard]] time_t GetExpiry() const
	{
		return i_expiryTime;
	}
private:
	time_t i_expiryTime;
};

struct TimeTrackerSmall
{
public:
	TimeTrackerSmall(int32 expiry = 0)
		: i_expiryTime(expiry)
	{
	}

	void Update(int32 diff)
	{
		i_expiryTime -= diff;
	}

	[[nodiscard]] bool Passed() const
	{
		return i_expiryTime <= 0;
	}

	void Reset(int32 interval)
	{
		i_expiryTime = interval;
	}

	[[nodiscard]] int32 GetExpiry() const
	{
		return i_expiryTime;
	}
private:
	int32 i_expiryTime;
};

struct PeriodicTimer
{
public:
	PeriodicTimer(int32 period, int32 start_time)
		: i_period(period), i_expireTime(start_time)
	{
	}

	bool Update(const uint32 diff)
	{
		if ((i_expireTime -= diff) > 0)
		{
			return false;
		}

		i_expireTime += i_period > int32(diff) ? i_period : diff;
		return true;
	}

	void SetPeriodic(int32 period, int32 start_time)
	{
		i_expireTime = start_time;
		i_period = period;
	}

	void TUpdate(int32 diff) { i_expireTime -= diff; }
	[[nodiscard]] bool TPassed() const { return i_expireTime <= 0; }
	void TReset(int32 diff, int32 period) { i_expireTime += period > diff ? period : diff; }
private:
	int32 i_period;
	int32 i_expireTime;
};


#endif