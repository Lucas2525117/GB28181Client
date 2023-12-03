// ʱ����غ���
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

// ��ȡϵͳʱ��(system_clock)
ZDTIME ZD_GetSystemTime_S();         // ��ȷ����
ZDTIME ZD_GetSystemTime_MS();        // ��ȷ������
ZDTIME ZD_GetSystemTime_MicroS();    // ��ȷ��΢��
ZDTIME ZD_GetSystemTime_NS();        // ��ȷ������

// ��ǰʱ��ת��Ϊ�ַ��� ����:ת������ַ���Ϊ"2023-11-23 12:01:35"
const char* ZD_FormatCurrentTime();

// �ض�ʱ��ת��Ϊ�ַ�����flag�ο�ZD_TIMEFLAG����
const char* ZD_FormatTime(ZDTIME zt, int flag);

#endif

