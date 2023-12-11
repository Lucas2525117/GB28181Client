// �豸����(������)
#ifndef _GB_DEVICECONFIG_H_
#define _GB_DEVICECONFIG_H_

#include <string>
#include "MyGBDeviceManager.h"
#include "MyGBDevice.h"
#include "MySipContext.h"
#include "MySipInfo.h"
#include "json/json.h"

class CGBDeviceConfig
{
public:
	static CGBDeviceConfig& GetInstance()
	{
		static CGBDeviceConfig gbDeviceConfig;
		return gbDeviceConfig;
	}

	int SetCommand(const std::string& gbid, const std::string& method, const std::string& request);

private:
	static std::string GetConfigSN()
	{
		static std::recursive_mutex mutex_;
		static unsigned int m_sn = 1;
		std::ostringstream sn;

		RecursiveGuard mtx(mutex_);
		sn << m_sn++;
		return sn.str();
	}

	// ������������
	int BaseParamConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request);

	// SVAC��������
	int SVACEncodeConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request);

	// SVAC��������
	int SVACDecodeConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request);

	// ��Ƶ������������
	int VideoParamPropertyConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request);

	// ¼��ƻ�����
	int RecordPlanConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request);

	// ����¼������
	int AlarmRecordConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request);

	// ��Ƶ�����ڵ�����
	int PictureMaskConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request);

	// ���淭ת����
	int FrameMirrorConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request);

	// �����ϱ���������
	int AlarmReportConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request);

	// ǰ��OSD����
	int OSDConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request);

	// ͼ��ץ������
	int SnapShotConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request);

private:
	CGBDeviceConfig();
	~CGBDeviceConfig();

private:
	typedef std::lock_guard<std::recursive_mutex> RecursiveGuard;
};

#endif
