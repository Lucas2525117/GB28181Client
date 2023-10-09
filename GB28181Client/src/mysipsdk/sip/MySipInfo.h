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

class CMyDeviceInfo
{
public:
	CMyDeviceInfo()
	{
	}

	~CMyDeviceInfo()
	{
	}

	CMyDeviceInfo(const CMyDeviceInfo& rhs)
	{
		deviceID = rhs.deviceID;
		deviceName = rhs.deviceName;
		result = rhs.result;
		deviceType = rhs.deviceType;
		manufacturer = rhs.manufacturer;
		model = rhs.model;
		firmware = rhs.firmware;
		maxCamera = rhs.maxCamera;
		maxAlarm = rhs.maxAlarm;
		channel = rhs.channel;
		port = rhs.port;
	}

	CMyDeviceInfo& operator=(const CMyDeviceInfo& rhs)
	{
		if (this == &rhs)
			return *this;
		
		deviceID = rhs.deviceID;
		deviceName = rhs.deviceName;
		result = rhs.result;
		deviceType = rhs.deviceType;
		manufacturer = rhs.manufacturer;
		model = rhs.model;
		firmware = rhs.firmware;
		maxCamera = rhs.maxCamera;
		maxAlarm = rhs.maxAlarm;
		channel = rhs.channel;
		port = rhs.port;
		return *this;
	}

	std::string     deviceID = "";
	std::string     deviceName = "";
	std::string     result = "";
	std::string     deviceType = "";
	std::string     manufacturer = "";  // 设备生产商
	std::string     model = "";         // 设备型号
	std::string     firmware = "";      // 设备固件版本
	std::string     maxCamera = "";     
	std::string     maxAlarm = "";
	std::string     channel = "";
	std::string     port = ""; 
};

class CMyDeviceStatus
{
public:
	CMyDeviceStatus()
	{
	}

	~CMyDeviceStatus()
	{
	}

	CMyDeviceStatus(const CMyDeviceStatus& rhs)
	{
		deviceID = rhs.deviceID;
		online = rhs.online;
		status = rhs.status;
		result = rhs.result;
		deviceTime = rhs.deviceTime;
		encode = rhs.encode;
		record = rhs.record;
	}

	CMyDeviceStatus& operator=(const CMyDeviceStatus& rhs)
	{
		if (this == &rhs)
			return *this;

		deviceID = rhs.deviceID;
		online = rhs.online;
		status = rhs.status;
		result = rhs.result;
		deviceTime = rhs.deviceTime;
		encode = rhs.encode;
		record = rhs.record;
		return *this;
	}

	std::string deviceID = "";
	std::string online = "";
	std::string status = "";
	std::string result = "";
	std::string deviceTime = "";
	std::string encode = "";
	std::string record = "";
	// todo ---> Alarmstatus:报警状态
};

class CMyRecordInfo
{
public:
	CMyRecordInfo()
	{
	}

	~CMyRecordInfo()
	{
	}

	CMyRecordInfo(const CMyRecordInfo& rhs)
	{
		deviceID = rhs.deviceID;
		deviceName = rhs.deviceName;
		filePath = rhs.filePath;
		address = rhs.address;
		startTime = rhs.startTime;
		endTime = rhs.endTime;
		fileSize = rhs.fileSize;
		//secrecy = rhs.secrecy;
		//type = rhs.type;
	}

	CMyRecordInfo& operator=(const CMyRecordInfo& rhs)
	{
		if (this == &rhs)
			return *this;

		deviceID = rhs.deviceID;
		deviceName = rhs.deviceName;
		filePath = rhs.filePath;
		address = rhs.address;
		startTime = rhs.startTime;
		endTime = rhs.endTime;
		fileSize = rhs.fileSize;
		//secrecy = rhs.secrecy;
		//type = rhs.type;
		return *this;
	}

	std::string   deviceID = "";
	std::string   deviceName = "";
	std::string   filePath = "";
	std::string   address = "";
	std::string   startTime = "";
	std::string   endTime = "";
	std::string   fileSize = "";
	//std::string   secrecy = "";
	//std::string   type = "";
};

#endif
