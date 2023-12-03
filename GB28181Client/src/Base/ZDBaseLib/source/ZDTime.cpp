#include "ZDTime.h"

#define ZD_CURRENT_TIME  (std::chrono::system_clock::now())

static inline const char* ZD_FormatTime_Mpl_(std::chrono::system_clock::time_point tp)
{
	std::time_t t = std::chrono::system_clock::to_time_t(tp);
	std::stringstream s;
	s << std::put_time(std::localtime(&t), "%F %T");
	return s.str().c_str();
}

ZDTIME ZD_GetSystemTime_S()
{
	return std::chrono::time_point_cast<std::chrono::seconds>(ZD_CURRENT_TIME).time_since_epoch().count();
}

ZDTIME ZD_GetSystemTime_MS()
{
	return std::chrono::time_point_cast<std::chrono::milliseconds>(ZD_CURRENT_TIME).time_since_epoch().count();
}

ZDTIME ZD_GetSystemTime_MicroS()
{
	return std::chrono::time_point_cast<std::chrono::microseconds>(ZD_CURRENT_TIME).time_since_epoch().count();
}

ZDTIME ZD_GetSystemTime_NS()
{
	return std::chrono::time_point_cast<std::chrono::nanoseconds>(ZD_CURRENT_TIME).time_since_epoch().count();
}

const char* ZD_FormatCurrentTime()
{
	return ZD_FormatTime_Mpl_(ZD_CURRENT_TIME);
}

const char* ZD_FormatTime(ZDTIME zt, int flag)
{
	std::chrono::system_clock::time_point tp;
	if (ZD_TIME_SECONDS == flag)
		tp = std::chrono::system_clock::time_point(std::chrono::seconds(zt));
	else if (ZD_TIME_MILLISECONDS == flag)
		tp = std::chrono::system_clock::time_point(std::chrono::milliseconds(zt));
	else if (ZD_TIME_MICROSECOND == flag)
		tp = std::chrono::system_clock::time_point(std::chrono::microseconds(zt));
	else
		return nullptr;

	return ZD_FormatTime_Mpl_(tp);
}