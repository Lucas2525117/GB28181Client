// …Ë±∏≈‰÷√(¥˝µ˜ ‘)
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

	// ª˘±æ≤Œ ˝≈‰÷√
	int BaseParamConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request);

	// SVAC±‡¬Î≈‰÷√
	int SVACEncodeConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request);

	// SVACΩ‚¬Î≈‰÷√
	int SVACDecodeConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request);

	//  ”∆µ≤Œ ˝ Ù–‘≈‰÷√
	int VideoParamPropertyConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request);

	// ¬ºœÒº∆ªÆ≈‰÷√
	int RecordPlanConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request);

	// ±®æØ¬ºœÒ≈‰÷√
	int AlarmRecordConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request);

	//  ”∆µª≠√Ê’⁄µ≤≈‰÷√
	int PictureMaskConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request);

	// ª≠√Ê∑≠◊™≈‰÷√
	int FrameMirrorConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request);

	// ±®æØ…œ±®ø™πÿ≈‰÷√
	int AlarmReportConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request);

	// «∞∂ÀOSD≈‰÷√
	int OSDConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request);

	// ÕºœÒ◊•≈ƒ≈‰÷√
	int SnapShotConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request);

private:
	CGBDeviceConfig();
	~CGBDeviceConfig();

private:
	typedef std::lock_guard<std::recursive_mutex> RecursiveGuard;
};

#endif
