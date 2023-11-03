#include "MyAlarmHandler.h"

CMyAlarmHandler::CMyAlarmHandler()
{

}

CMyAlarmHandler::~CMyAlarmHandler()
{
}

bool CMyAlarmHandler::OnReceive(pjsip_rx_data* rdata)
{
	if (PJSIP_OTHER_METHOD == rdata->msg_info.cseq->method.id)
	{
		CMyXmlParser xmlParser(CMySipContext::GetInstance().GetMessageBody(rdata));
		CMyDynamicStruct dynamicStruct;
		dynamicStruct.Set(xmlParser.GetXml());

		auto cmd = xmlParser.GetXml()->RootElement()->Value();
		auto cmdType = dynamicStruct.Get("CmdType");
		if ("Alarm" != cmdType)
			return false;

		CMyAlarmInfo alarmInfo;
		alarmInfo.sn = dynamicStruct.Get("SN");
		alarmInfo.deviceID = dynamicStruct.Get("DeviceID");
		alarmInfo.alarmPrority = (AlarmPriority)atoi(dynamicStruct.Get("AlarmPriority").c_str());
		alarmInfo.alarmMethod = (AlarmMethod)atoi(dynamicStruct.Get("AlarmPriority").c_str());
		alarmInfo.alarmTime = dynamicStruct.Get("AlarmTime");
		alarmInfo.alarmDescription = dynamicStruct.Get("AlarmDescription");

		if (m_dataCB)
			m_dataCB(m_handleType, m_user, &alarmInfo);

		Response(rdata, PJSIP_SC_OK, NoHead);

		// 报警通知响应
		char szAlarmInfo[200] = { 0 };
		pj_ansi_snprintf(szAlarmInfo, 200,
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
			"<Response>\n"
			"<CmdType>%s</CmdType>\n"
			"<SN>%s</SN>\n"
			"<DeviceID>%s</DeviceID>\n"
			"<Result>OK</Result>\n"
			"</Response>\n", cmdType.c_str(), alarmInfo.sn.c_str(), alarmInfo.deviceID.c_str()
		);
		Response(rdata, PJSIP_SC_OK, NoHead, szAlarmInfo, SUBTYPE_XML);
		return true;
	}

	return false;
}