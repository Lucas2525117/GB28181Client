#pragma once

#include <qstring.h>
#include <qstringlist.h>

enum NodeType
{
	TypeOrg = 0,			// ��֯
	TypeDevice,             // �豸
	TypeChannel,            // ͨ��
};

typedef struct GlobalConfigParam
{
	int streamTransMode;   // 0:UDP 1:TCP���� 2:TCP����

	GlobalConfigParam()
	{
		streamTransMode = 0;
	}
}GlobalConfigParam;

static bool ipAddrIsOK(const QString& ip)
{
	if (ip.isEmpty())
	{
		return false;
	}

	QStringList list = ip.split('.');
	if (list.size() != 4)
	{
		return false;
	}

	for (const auto& num : list)
	{
		bool ok = false;
		int temp = num.toInt(&ok);
		if (!ok || temp < 0 || temp > 255)
		{
			return false;
		}
	}

	return true;
}

inline time_t StringToDatetime(const char* str)
{
	tm tm_;
	int year, month, day, hour, minute, second;
	sscanf(str, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
	tm_.tm_year = year - 1900;
	tm_.tm_mon = month - 1;
	tm_.tm_mday = day;
	tm_.tm_hour = hour;
	tm_.tm_min = minute;
	tm_.tm_sec = second;
	tm_.tm_isdst = 0;

	time_t t_ = mktime(&tm_); //�Ѿ�����8��ʱ��  
	return t_; //��ʱ��  
}

