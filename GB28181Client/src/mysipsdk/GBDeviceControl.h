// 设备控制(待调试)
#ifndef _GB_DEVICECONTROL_H_
#define _GB_DEVICECONTROL_H_

#include <string>
#include "MyGBDeviceManager.h"
#include "MyGBDevice.h"
#include "MySipContext.h"
#include "MySipInfo.h"
#include "json/json.h"

class CGBDeviceControl
{
public:
	static CGBDeviceControl& GetInstance()
	{
		static CGBDeviceControl gbDeviceControl;
		return gbDeviceControl;
	}

	int SetCommand(const std::string& gbid, const std::string& method, const std::string& request);

private:
	static std::string GetControlSN()
	{
		static std::recursive_mutex mutex_;
		static unsigned int m_sn = 1;
		std::ostringstream sn;

		RecursiveGuard mtx(mutex_);
		sn << m_sn++;
		return sn.str();
	}

	// 云台控制
	int PTZControl_(CMyGBDevice* device, const std::string& deviceID, const std::string& request);

	// 远程启动
	int TeleRebootControl_(CMyGBDevice* device, const std::string& deviceID, const std::string& request);

	// 录像控制
	int RecordControl_(CMyGBDevice* device, const std::string& deviceID, const std::string& request);

	// 布防/撤防
	int GuardControl_(CMyGBDevice* device, const std::string& deviceID, const std::string& request);

	// 报警复位
	int AlarmResetControl_(CMyGBDevice* device, const std::string& deviceID, const std::string& request);

	// 强制关键帧(设备收到该命令立刻发送一个IDR帧)
	int IFrameControl_(CMyGBDevice* device, const std::string& deviceID, const std::string& request);

	// 拉框放大
	int DragZoomInControl_(CMyGBDevice* device, const std::string& deviceID, const std::string& request);

	// 拉框缩小
	int DragZoomOutControl_(CMyGBDevice* device, const std::string& deviceID, const std::string& request);

	// 看守位控制
	int HomePositionControl_(CMyGBDevice* device, const std::string& deviceID, const std::string& request);

	// PTZ精准控制
	int PTZPreciseControl_(CMyGBDevice* device, const std::string& deviceID, const std::string& request);

	// 设备软件升级
	int DeviceUpgradeControl_(CMyGBDevice* device, const std::string& deviceID, const std::string& request);

	// 存储卡格式化
	int FormatSDCardControl_(CMyGBDevice* device, const std::string& deviceID, const std::string& request);

	// 目标跟踪
	int TargetTrackControl_(CMyGBDevice* device, const std::string& deviceID, const std::string& request);

private:
	CGBDeviceControl();
	~CGBDeviceControl();

private:
	typedef std::lock_guard<std::recursive_mutex> RecursiveGuard;
};

#endif
