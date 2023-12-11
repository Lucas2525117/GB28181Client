#include "GBDeviceControl.h"

CGBDeviceControl::CGBDeviceControl()
{
}

CGBDeviceControl::~CGBDeviceControl()
{
}

int CGBDeviceControl::SetCommand(const std::string& gbid, const std::string& method, const std::string& request)
{
	MyGBDevicePtr device = CMyGBDeviceManager::GetInstance().GetDeviceById(gbid);
	if (!device.get())
		return -1;

	int ret = -1;
	if (0 == method.compare("PTZControl"))
		ret = PTZControl_(device.get(), gbid, request);
	else if(0 == method.compare("TeleRebootControl"))
		ret = TeleRebootControl_(device.get(), gbid, request);
	else if (0 == method.compare("RecordControl"))
		ret = RecordControl_(device.get(), gbid, request);
	else if (0 == method.compare("GuardControl"))
		ret = GuardControl_(device.get(), gbid, request);
	else if (0 == method.compare("AlarmResetControl"))
		ret = AlarmResetControl_(device.get(), gbid, request);
	else if (0 == method.compare("IFrameControl"))
		ret = IFrameControl_(device.get(), gbid, request);
	else if (0 == method.compare("DragZoomInControl"))
		ret = DragZoomInControl_(device.get(), gbid, request);
	else if (0 == method.compare("DragZoomOutControl"))
		ret = DragZoomOutControl_(device.get(), gbid, request);
	else if (0 == method.compare("HomePositionControl"))
		ret = HomePositionControl_(device.get(), gbid, request);
	else if (0 == method.compare("PTZPreciseControl"))
		ret = PTZPreciseControl_(device.get(), gbid, request);
	else if (0 == method.compare("DeviceUpgradeControl"))
		ret = DeviceUpgradeControl_(device.get(), gbid, request);
	else if (0 == method.compare("FormatSDCardControl"))
		ret = FormatSDCardControl_(device.get(), gbid, request);
	else if (0 == method.compare("TargetTrackControl_"))
		ret = TargetTrackControl_(device.get(), gbid, request);

	return ret;
}

int CGBDeviceControl::PTZControl_(CMyGBDevice* device, const std::string& deviceID, const std::string& request)
{
	return 0;
}

int CGBDeviceControl::TeleRebootControl_(CMyGBDevice* device, const std::string& deviceID, const std::string& request)
{
	if (deviceID.empty())
		return -1;

	std::string sn = GetControlSN();
	char teleBootInfo[200] = { 0 };
	snprintf(teleBootInfo, 200,
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<Control>\n"
		"<CmdType>DeviceControl</CmdType>\n"
		"<SN>%s</SN>\n"
		"<DeviceID>%s</DeviceID>\n"
		"<TeleBoot>Boot</TeleBoot>\n"
		"</Control>\n"
		, sn.c_str()
		, deviceID.c_str()
	);

	return CMySipContext::GetInstance().SendSipMessage(device, teleBootInfo);
}

int CGBDeviceControl::RecordControl_(CMyGBDevice* device, const std::string& deviceID, const std::string& request)
{
	return 0;
}

int CGBDeviceControl::GuardControl_(CMyGBDevice* device, const std::string& deviceID, const std::string& request)
{
	if (!device || deviceID.empty() || request.empty())
		return -1;

	Json::Value value;
	Json::Reader reader;
	if (!reader.parse(request.c_str(), value))
		return -1;

	std::string sn = GetControlSN();
	std::string guardCmd = "";
	if (0 == value["cmd"].asString().compare("1"))  // ²¼·À
		guardCmd = "SetGuard";
	else                                            // ³··À
		guardCmd = "ResetGuard";

	char guardInfo[200] = { 0 };
	snprintf(guardInfo, 200,
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<Control>\n"
		"<CmdType>DeviceControl</CmdType>\n"
		"<SN>%s</SN>\n"
		"<DeviceID>%s</DeviceID>\n"
		"<GuardCmd>%s</GuardCmd>\n"
		"</Control>\n"
		, sn.c_str()
		, deviceID.c_str()
		, guardCmd.c_str()
	);

	return CMySipContext::GetInstance().SendSipMessage(device, guardInfo);
}

int CGBDeviceControl::AlarmResetControl_(CMyGBDevice* device, const std::string& deviceID, const std::string& request)
{
	if (!device || deviceID.empty() || request.empty())
		return -1;

	Json::Value value;
	Json::Reader reader;
	if (!reader.parse(request.c_str(), value))
		return -1;

	std::string sn = GetControlSN();
	std::string alarmMethod = value["AlarmMethod"].asString();
	std::string alarmType = value["AlarmType"].asString();
	
	char alarmResetInfo[400] = { 0 };
	snprintf(alarmResetInfo, 400,
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<Control>\n"
		"<CmdType>DeviceControl</CmdType>\n"
		"<SN>%s</SN>\n"
		"<DeviceID>%s</DeviceID>\n"
		"<AlarmCmd>ResetAlarm</AlarmCmd>\n"
		"<Info>\n"
		"<AlarmMethod>%s</AlarmMethod>\n"
		"<AlarmType>%s</AlarmType>\n"
		"</Info>\n"
		"</Control>\n"
		, sn.c_str()
		, deviceID.c_str()
		, alarmMethod.c_str()
		, alarmType.c_str()
	);

	return CMySipContext::GetInstance().SendSipMessage(device, alarmResetInfo);
}

int CGBDeviceControl::IFrameControl_(CMyGBDevice* device, const std::string& deviceID, const std::string& request)
{
	if (!device || deviceID.empty())
		return -1;

	std::string sn = GetControlSN();
	char iframeInfo[200] = { 0 };
	snprintf(iframeInfo, 200,
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<Control>\n"
		"<CmdType>DeviceControl</CmdType>\n"
		"<SN>%s</SN>\n"
		"<DeviceID>%s</DeviceID>\n"
		"<IFrameCmd>Send</IFrameCmd>\n"
		"</Control>\n"
		, sn.c_str()
		, deviceID.c_str()
	);

	return CMySipContext::GetInstance().SendSipMessage(device, iframeInfo);
}

int CGBDeviceControl::DragZoomInControl_(CMyGBDevice* device, const std::string& deviceID, const std::string& request)
{
	if (!device || deviceID.empty() || request.empty())
		return -1;

	Json::Value value;
	Json::Reader reader;
	if (!reader.parse(request.c_str(), value))
		return -1;

	std::string sn = GetControlSN();
	int length = value["Length"].asInt();
	int width = value["Width"].asInt();
	int midPointX = value["MidPointX"].asInt();
	int midPointY = value["MidPointY"].asInt();
	int lengthX = value["LengthX"].asInt();
	int lengthY = value["LengthY"].asInt();

	char dragZoomInInfo[500] = { 0 };
	snprintf(dragZoomInInfo, 500,
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<Control>\n"
		"<CmdType>DeviceControl</CmdType>\n"
		"<SN>%s</SN>\n"
		"<DeviceID>%s</DeviceID>\n"
		"<DragZoomIn>\n"
		"<Length>%d</Length>\n"
		"<Width>%d</Width>\n"
		"<MidPointX>%d</MidPointX>\n"
		"<MidPointY>%d</MidPointY>\n"
		"<LengthX>%d</LengthX>\n"
		"<LengthY>%d</LengthY>\n"
		"</DragZoomIn>\n"
		"</Control>\n"
		, sn.c_str()
		, deviceID.c_str()
		, length
		, width
		, midPointX
		, midPointY
		, lengthX
		, lengthY
	);

	return CMySipContext::GetInstance().SendSipMessage(device, dragZoomInInfo);
}

int CGBDeviceControl::DragZoomOutControl_(CMyGBDevice* device, const std::string& deviceID, const std::string& request)
{
	if (!device || deviceID.empty() || request.empty())
		return -1;

	Json::Value value;
	Json::Reader reader;
	if (!reader.parse(request.c_str(), value))
		return -1;

	std::string sn = GetControlSN();
	int length = value["Length"].asInt();
	int width = value["Width"].asInt();
	int midPointX = value["MidPointX"].asInt();
	int midPointY = value["MidPointY"].asInt();
	int lengthX = value["LengthX"].asInt();
	int lengthY = value["LengthY"].asInt();

	char dragZoomOutInfo[500] = { 0 };
	snprintf(dragZoomOutInfo, 500,
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<Control>\n"
		"<CmdType>DeviceControl</CmdType>\n"
		"<SN>%s</SN>\n"
		"<DeviceID>%s</DeviceID>\n"
		"<DragZoomOut>\n"
		"<Length>%d</Length>\n"
		"<Width>%d</Width>\n"
		"<MidPointX>%d</MidPointX>\n"
		"<MidPointY>%d</MidPointY>\n"
		"<LengthX>%d</LengthX>\n"
		"<LengthY>%d</LengthY>\n"
		"</DragZoomOut>\n"
		"</Control>\n"
		, sn.c_str()
		, deviceID.c_str()
		, length
		, width
		, midPointX
		, midPointY
		, lengthX
		, lengthY
	);

	return CMySipContext::GetInstance().SendSipMessage(device, dragZoomOutInfo);
}

int CGBDeviceControl::HomePositionControl_(CMyGBDevice* device, const std::string& deviceID, const std::string& request)
{
	if (!device || deviceID.empty() || request.empty())
		return -1;

	Json::Value value;
	Json::Reader reader;
	if (!reader.parse(request.c_str(), value))
		return -1;

	std::string sn = GetControlSN();
	int enabled = value["Enabled"].asInt();
	int resetTime = value["ResetTime"].asInt();
	int presetIndex = value["PresetIndex"].asInt();

	char homePositionInfo[500] = { 0 };
	snprintf(homePositionInfo, 500,
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<Control>\n"
		"<CmdType>DeviceControl</CmdType>\n"
		"<SN>%s</SN>\n"
		"<DeviceID>%s</DeviceID>\n"
		"<HomePosition>\n"
		"<Enabled>%d</Enabled>\n"
		"<ResetTime>%d</ResetTime>\n"
		"<PresetIndex>%d</PresetIndex>\n"
		"</HomePosition>\n"
		"</Control>\n"
		, sn.c_str()
		, deviceID.c_str()
		, enabled
		, resetTime
		, presetIndex
	);

	return CMySipContext::GetInstance().SendSipMessage(device, homePositionInfo);
}

int CGBDeviceControl::PTZPreciseControl_(CMyGBDevice* device, const std::string& deviceID, const std::string& request)
{
	return 0;
}

int CGBDeviceControl::DeviceUpgradeControl_(CMyGBDevice* device, const std::string& deviceID, const std::string& request)
{
	return 0;
}

int CGBDeviceControl::FormatSDCardControl_(CMyGBDevice* device, const std::string& deviceID, const std::string& request)
{
	return 0;
}

int CGBDeviceControl::TargetTrackControl_(CMyGBDevice* device, const std::string& deviceID, const std::string& request)
{
	return 0;
}
