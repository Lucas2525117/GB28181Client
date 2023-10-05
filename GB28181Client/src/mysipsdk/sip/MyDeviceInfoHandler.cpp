#include "MyDeviceInfoHandler.h"

CMyDeviceInfoHandler::CMyDeviceInfoHandler()
{

}

CMyDeviceInfoHandler::~CMyDeviceInfoHandler()
{
}

bool CMyDeviceInfoHandler::OnReceive(pjsip_rx_data* rdata)
{
	if (PJSIP_OTHER_METHOD == rdata->msg_info.cseq->method.id)
	{
		CMyXmlParser xmlParser(CMySipContext::GetInstance().GetMessageBody(rdata));
		CMyDynamicStruct dynamicStruct;
		dynamicStruct.Set(xmlParser.GetXml());

		auto cmd = xmlParser.GetXml()->RootElement()->Value();
		auto cmdType = dynamicStruct.Get("CmdType");
		if ("DeviceInfo" != cmdType)
			return false;

		CMyDeviceInfo deviceInfo;
		deviceInfo.deviceID = dynamicStruct.Get("DeviceID");
		deviceInfo.deviceName = dynamicStruct.Get("DeviceName");
		deviceInfo.result = dynamicStruct.Get("Result");
		deviceInfo.deviceType = dynamicStruct.Get("DeviceType");
		deviceInfo.manufacturer = dynamicStruct.Get("Manufacturer");
		deviceInfo.model = dynamicStruct.Get("Model");
		deviceInfo.firmware = dynamicStruct.Get("Firmware");
		deviceInfo.maxCamera = dynamicStruct.Get("MaxCamera");
		deviceInfo.maxAlarm = dynamicStruct.Get("MaxAlarm");
		deviceInfo.channel = dynamicStruct.Get("Channel");
		deviceInfo.port = dynamicStruct.Get("Port");

		if (m_dataCB)
			m_dataCB(m_handleType, m_user, &deviceInfo);

		Response(rdata, PJSIP_SC_OK, NoHead);
		return true;
	}

	return false;
}