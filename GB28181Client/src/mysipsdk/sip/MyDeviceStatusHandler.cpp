#include "MyDeviceStatusHandler.h"

CMyDeviceStatusHandler::CMyDeviceStatusHandler()
{
}

CMyDeviceStatusHandler::~CMyDeviceStatusHandler()
{
}

bool CMyDeviceStatusHandler::OnReceive(pjsip_rx_data* rdata)
{
	if (PJSIP_OTHER_METHOD == rdata->msg_info.cseq->method.id)
	{
		CMyXmlParser xmlParser(CMySipContext::GetInstance().GetMessageBody(rdata));
		CMyDynamicStruct dynamicStruct;
		dynamicStruct.Set(xmlParser.GetXml());

		auto cmd = xmlParser.GetXml()->RootElement()->Value();
		auto cmdType = dynamicStruct.Get("CmdType");
		if ("DeviceStatus" != cmdType)
			return false;

		CMyDeviceStatus deviceStatus;
		deviceStatus.deviceID = dynamicStruct.Get("DeviceID");
		deviceStatus.online = dynamicStruct.Get("Online");
		deviceStatus.status = dynamicStruct.Get("Status");
		deviceStatus.result = dynamicStruct.Get("Result");
		deviceStatus.deviceTime = dynamicStruct.Get("DeviceTime");
		deviceStatus.encode = dynamicStruct.Get("Encode");
		deviceStatus.record = dynamicStruct.Get("Record");

		if (m_dataCB)
			m_dataCB(m_handleType, m_user, &deviceStatus);

		Response(rdata, PJSIP_SC_OK, NoHead);
		return true;
	}

	return false;
}
