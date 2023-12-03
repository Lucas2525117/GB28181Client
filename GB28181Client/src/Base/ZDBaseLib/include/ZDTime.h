// 时间相关函数
#ifndef _ZD_TIME_H_
#define _ZD_TIME_H_

#if defined(_MSC_VER) && _MSC_VER>1000
	#pragma once
#endif

#include <chrono>
#include <iomanip>
#include <sstream>

typedef int64_t ZDTIME;

enum ZD_TIMEFLAG
{
	ZD_TIME_SECONDS = 0,
	ZD_TIME_MILLISECONDS,
	ZD_TIME_MICROSECOND, 
};

// 获取系统时间(system_clock)
ZDTIME ZD_GetSystemTime_S();         // 精确至秒
ZDTIME ZD_GetSystemTime_MS();        // 精确至毫秒
ZDTIME ZD_GetSystemTime_MicroS();    // 精确至微秒
ZDTIME ZD_GetSystemTime_NS();        // 精确至纳秒

// 当前时间转换为字符串 例如:转换后的字符串为"2023-11-23 12:01:35"
const char* ZD_FormatCurrentTime();

// 特定时间转换为字符串，flag参考ZD_TIMEFLAG定义
const char* ZD_FormatTime(ZDTIME zt, int flag);

#endif

