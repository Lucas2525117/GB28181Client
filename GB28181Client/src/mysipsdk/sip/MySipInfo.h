#ifndef _MY_SIPINFO_H_
#define _MY_SIPINFO_H_

#include <string>

class CMyCatalogInfo
{
public:
	CMyCatalogInfo()
		: Address("")
		, DeviceID("")
		, IPAddress("")
		, componentid("")
		, Name("")
		, Manufacturer("")
		, Model("")
		, Owner("")
		, Civilcode("")
		, Safetyway(0)
		, Registerway("")
		, Secrecy("")
		, lastAccessTime(0)
		, Status("")
	{
	}

	~CMyCatalogInfo()
	{
	}

	CMyCatalogInfo(const CMyCatalogInfo& rhs)
	{
		Address = rhs.Address;
		DeviceID = rhs.DeviceID;
		IPAddress = rhs.IPAddress;
		componentid = rhs.componentid;
		Name = rhs.Name;
		Manufacturer = rhs.Manufacturer;
		Model = rhs.Model;
		Owner = rhs.Owner;
		Civilcode = rhs.Civilcode;
		Safetyway = rhs.Safetyway;
		Registerway = rhs.Registerway;
		Secrecy = rhs.Secrecy;
		PlatformAddr = rhs.PlatformAddr;
		PlatformPort = rhs.PlatformPort;
		lastAccessTime = rhs.lastAccessTime;
		Status = rhs.Status;
	}

	CMyCatalogInfo& operator=(const CMyCatalogInfo& rhs)
	{
		if (this == &rhs) 
			return *this;

		Address = rhs.Address;
		DeviceID = rhs.DeviceID;
		IPAddress = rhs.IPAddress;
		componentid = rhs.componentid;
		Name = rhs.Name;
		Manufacturer = rhs.Manufacturer;
		Model = rhs.Model;
		Owner = rhs.Owner;
		Civilcode = rhs.Civilcode;
		Safetyway = rhs.Safetyway;
		Registerway = rhs.Registerway;
		Secrecy = rhs.Secrecy;
		PlatformAddr = rhs.PlatformAddr;
		PlatformPort = rhs.PlatformPort;
		lastAccessTime = rhs.lastAccessTime;
		Status = rhs.Status;
		return *this;
	}

	std::string Address;
	std::string DeviceID;
	std::string IPAddress;
	std::string componentid;
	std::string Name;
	std::string Manufacturer;
	std::string Model;
	std::string Owner;
	std::string Civilcode;
	int Safetyway;
	std::string Registerway;
	std::string Secrecy;
	std::string PlatformAddr;
	int PlatformPort;
	long long lastAccessTime;
	std::string Status;
};

typedef struct CMyDeviceInfo
{
	std::string     deviceID;
	std::string     deviceName;
	std::string     result;
	std::string     deviceType;
	std::string     manufacturer;  // 设备生产商
	std::string     model;         // 设备型号
	std::string     firmware;      // 设备固件版本
	std::string     maxCamera;     
	std::string     maxAlarm;
	std::string     channel;
	std::string     port; 

	CMyDeviceInfo()
	{
		deviceID = "";
		deviceName = "";
		result = "";
		deviceType = "";
		manufacturer = "";  // 设备生产商
		model = "";         // 设备型号
		firmware = "";      // 设备固件版本
		maxCamera = "";
		maxAlarm = "";
		channel = "";
		port = "";
	}
}CMyDeviceInfo;

typedef struct CMyDeviceStatus
{
	std::string deviceID;
	std::string online;
	std::string status;
	std::string result;
	std::string deviceTime;
	std::string encode;
	std::string record;
	// todo ---> Alarmstatus:报警状态

	CMyDeviceStatus()
	{
		deviceID = "";
		online = "";
		status = "";
		result = "";
		deviceTime = "";
		encode = "";
		record = "";
	}
}CMyDeviceStatus;

typedef struct CMyRecordInfo
{
	std::string   deviceID;
	std::string   deviceName;
	std::string   filePath;
	std::string   address;
	std::string   startTime;
	std::string   endTime;
	std::string   fileSize;
	//std::string   secrecy = "";
	//std::string   type = "";

	CMyRecordInfo()
	{
		deviceID = "";
		deviceName = "";
		filePath = "";
		address = "";
		startTime = "";
		endTime = "";
		fileSize = "";
	}
}CMyRecordInfo;

typedef struct CMyDownloadInfo
{
	std::string deviceID;
	std::string notifyType;

	CMyDownloadInfo()
	{
		deviceID = "";
		notifyType = "";
	}
}CMyDownloadInfo;

enum AlarmPriority
{
	AlarmPriority_Undefined = 0, // 未定义
	AlarmPriority_One,           // 一级警情
	AlarmPriority_Two,           // 二级警情
	AlarmPriority_Three,         // 三级警情
	AlarmPriority_Four,          // 四级警情
};

enum AlarmMethod
{
	Alarm_Undefined = 0,         // 未定义
	Alarm_Phone,                 // 电话告警
	Alarm_Device,                // 设备告警
	Alarm_TextMessage,           // 短信告警
	Alarm_GPS,                   // GPS告警
	Alarm_Video,                 // 视频告警
	Alarm_DeviceBreak,           // 设备故障告警
	Alarm_Other,                 // 其他告警
};

typedef struct CMyAlarmInfo
{
	std::string       sn;               // 序列号
	std::string       deviceID;         // 报警设备编码或报警中心编码
	AlarmPriority     alarmPrority;     // 报警级别
	AlarmMethod       alarmMethod;      // 报警方式
	std::string       alarmTime;        // 报警时间
	std::string       alarmDescription; // 报警描述

	CMyAlarmInfo()
	{
		sn = "";
		deviceID = "";
		alarmPrority = AlarmPriority_Undefined;
		alarmMethod = Alarm_Undefined;
		alarmTime = "";
		alarmDescription = "";
	}
}CMyAlarmInfo;

typedef struct CMyBroadcastInfo
{
	std::string deviceID;    // 语音输出设备的设备编码
	std::string result;

	CMyBroadcastInfo()
	{
		deviceID = "";
		result = "";
	}
}CMyBroadcastInfo;

typedef struct CMyVideoInviteInfo
{
	std::string deviceIP;
	int transport;

	CMyVideoInviteInfo()
	{
		deviceIP = "";
		transport = 0;
	}
}CMyVideoInviteInfo;

#endif
