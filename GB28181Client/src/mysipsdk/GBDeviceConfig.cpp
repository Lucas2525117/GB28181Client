#include "GBDeviceConfig.h"

CGBDeviceConfig::CGBDeviceConfig()
{
}

CGBDeviceConfig::~CGBDeviceConfig()
{
}

int CGBDeviceConfig::SetCommand(const std::string& gbid, const std::string& method, const std::string& request)
{
	MyGBDevicePtr device = CMyGBDeviceManager::GetInstance().GetDeviceById(gbid);
	if (!device.get())
		return -1;

	int ret = -1;
	if (0 == method.compare("SnapShotConfig"))
		ret = SnapShotConfig_(device.get(), gbid, request);
	else if (0 == method.compare("BaseParamConfig"))
		ret = BaseParamConfig_(device.get(), gbid, request);
	else if(0 == method.compare("SVACEncodeConfig"))
		ret = SVACEncodeConfig_(device.get(), gbid, request);
	else if (0 == method.compare("SVACDecodeConfig"))
		ret = SVACDecodeConfig_(device.get(), gbid, request);
	else if (0 == method.compare("VideoParamPropertyConfig"))
		ret = VideoParamPropertyConfig_(device.get(), gbid, request);
	else if (0 == method.compare("RecordPlanConfig"))
		ret = RecordPlanConfig_(device.get(), gbid, request);
	else if (0 == method.compare("AlarmRecordConfig"))
		ret = AlarmRecordConfig_(device.get(), gbid, request);
	else if (0 == method.compare("PictureMaskConfig"))
		ret = PictureMaskConfig_(device.get(), gbid, request);
	else if (0 == method.compare("FrameMirrorConfig"))
		ret = FrameMirrorConfig_(device.get(), gbid, request);
	else if (0 == method.compare("AlarmReportConfig"))
		ret = AlarmReportConfig_(device.get(), gbid, request);
	else if (0 == method.compare("OSDConfig"))
		ret = OSDConfig_(device.get(), gbid, request);

	return ret;
}

int CGBDeviceConfig::BaseParamConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request)
{
	return 0;
}

int CGBDeviceConfig::SVACEncodeConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request)
{
	return 0;
}

int CGBDeviceConfig::SVACDecodeConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request)
{
	return 0;
}

int CGBDeviceConfig::VideoParamPropertyConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request)
{
	return 0;
}

int CGBDeviceConfig::RecordPlanConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request)
{
	return 0;
}

int CGBDeviceConfig::AlarmRecordConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request)
{
	return 0;
}

int CGBDeviceConfig::PictureMaskConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request)
{
	return 0;
}

int CGBDeviceConfig::FrameMirrorConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request)
{
	return 0;
}

int CGBDeviceConfig::AlarmReportConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request)
{
	return 0;
}

int CGBDeviceConfig::OSDConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request)
{
	if (!device || gbid.empty() || request.empty())
		return -1;

	Json::Value value;
	Json::Reader reader;
	if (!reader.parse(request.c_str(), value))
		return -1;

	std::string sn = GetConfigSN();
	int length = value["length"].asInt();
	int width = value["width"].asInt();
	int timeX = value["timeX"].asInt();
	int timeY = value["timeY"].asInt();
	int timeEnable = value["timeEnable"].asInt();
	int timeType = value["timeType"].asInt();
	int textEnable = value["textEnable"].asInt();
	int sumNum = value["textEnable"].asInt();

	std::list<CMyOSDText> osdTexts;
	for (auto it = value["osdText"].begin(); it != value["osdText"].end(); ++it)
	{
		CMyOSDText osdText;
		Json::Value& item = *it;
		osdText.text = item["test"].asString();
		osdText.x = item["x"].asInt();
		osdText.y = item["y"].asInt();
		osdTexts.push_back(osdText);
	}

	std::ostringstream content;
	for (auto it : osdTexts)
	{
		CMyOSDText& osdText = it;
		content << "<Item>";
		content << "<Text>" << osdText.text << "</Text>";
		content << "<X>" << osdText.x << "</X>";
		content << "<Y>" << osdText.y << "</Y>";
		content << "</Item>";
	}

	char osdInfo[500] = { 0 };
	snprintf(osdInfo, 500,
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<Control>\n"
		"<CmdType>DeviceConfig</CmdType>\n"
		"<SN>%s</SN>\n"
		"<DeviceID>%s</DeviceID>\n"
		"<OSDConfig>\n"
		"<Length>%d</Length>\n"
		"<Width>%d</Width>\n"
		"<TimeX>%d</TimeX>\n"
		"<TimeY>%d</TimeY>\n"
		"<TimeEnable>%d</TimeEnable>\n"
		"<TimeType>%d</TimeType>\n"
		"<TextEnable>%d</TextEnable>\n"
		"<SumNum>%d</SumNum>\n"
		"%s/n"
		"</OSDConfig>\n"
		"</Control>\n"
		, sn.c_str()
		, gbid.c_str()
		, length
		, width
		, timeX
		, timeY
		, timeEnable
		, timeType
		, textEnable
		, sumNum
		, content.str()
	);

	return CMySipContext::GetInstance().SendSipMessage(device, osdInfo);
}

int CGBDeviceConfig::SnapShotConfig_(CMyGBDevice* device, const std::string& gbid, const std::string& request)
{
	if (!device || gbid.empty() || request.empty())
		return -1;

	Json::Value value;
	Json::Reader reader;
	if (!reader.parse(request.c_str(), value))
		return -1;

	std::string sn = GetConfigSN();
	std::string uploadURL = value["uploadURL"].asString();
	std::string sessionID = value["sessionID"].asString();
	int snapNum = value["snapNum"].asInt();
	int interval = value["interval"].asInt();

	char snapShotInfo[500] = { 0 };
	snprintf(snapShotInfo, 500,
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<Control>\n"
		"<CmdType>DeviceConfig</CmdType>\n"
		"<SN>%s</SN>\n"
		"<DeviceID>%s</DeviceID>\n"
		"<SnapShotConfig>\n"
		"<SnapNum>%d</SnapNum>\n"
		"<Interval>%d</Interval>\n"
		"<UploadURL>%s</UploadURL>\n"
		"<SessionID>%s</SessionID>\n"
		"</SnapShotConfig>\n"
		"</Control>\n"
		, sn.c_str()
		, gbid.c_str()
		, snapNum
		, interval
		, uploadURL.c_str()
		, sessionID.c_str()
	);

	return CMySipContext::GetInstance().SendSipMessage(device, snapShotInfo);
}
