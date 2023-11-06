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
	std::string     manufacturer;  // �豸������
	std::string     model;         // �豸�ͺ�
	std::string     firmware;      // �豸�̼��汾
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
		manufacturer = "";  // �豸������
		model = "";         // �豸�ͺ�
		firmware = "";      // �豸�̼��汾
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
	// todo ---> Alarmstatus:����״̬

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
	AlarmPriority_Undefined = 0, // δ����
	AlarmPriority_One,           // һ������
	AlarmPriority_Two,           // ��������
	AlarmPriority_Three,         // ��������
	AlarmPriority_Four,          // �ļ�����
};

enum AlarmMethod
{
	Alarm_Undefined = 0,         // δ����
	Alarm_Phone,                 // �绰�澯
	Alarm_Device,                // �豸�澯
	Alarm_TextMessage,           // ���Ÿ澯
	Alarm_GPS,                   // GPS�澯
	Alarm_Video,                 // ��Ƶ�澯
	Alarm_DeviceBreak,           // �豸���ϸ澯
	Alarm_Other,                 // �����澯
};

typedef struct CMyAlarmInfo
{
	std::string       sn;               // ���к�
	std::string       deviceID;         // �����豸����򱨾����ı���
	AlarmPriority     alarmPrority;     // ��������
	AlarmMethod       alarmMethod;      // ������ʽ
	std::string       alarmTime;        // ����ʱ��
	std::string       alarmDescription; // ��������

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
	std::string deviceID;    // ��������豸���豸����
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
