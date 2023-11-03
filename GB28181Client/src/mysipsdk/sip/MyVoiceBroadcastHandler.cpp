#include "MyVoiceBroadcastHandler.h"

CMyVoiceBroadcastHandler::CMyVoiceBroadcastHandler()
{
}

CMyVoiceBroadcastHandler::~CMyVoiceBroadcastHandler()
{
}

bool CMyVoiceBroadcastHandler::OnReceive(pjsip_rx_data* rdata)
{
	if (PJSIP_OTHER_METHOD == rdata->msg_info.cseq->method.id)
	{
		CMyXmlParser xmlParser(CMySipContext::GetInstance().GetMessageBody(rdata));
		CMyDynamicStruct dynamicStruct;
		dynamicStruct.Set(xmlParser.GetXml());

		auto cmd = xmlParser.GetXml()->RootElement()->Value();
		auto cmdType = dynamicStruct.Get("CmdType");
		if ("Broadcast" != cmdType)
			return false;

		CMyBroadcastInfo broadcastInfo;
		broadcastInfo.deviceID = dynamicStruct.Get("DeviceID");
		broadcastInfo.result = dynamicStruct.Get("Result");

		if (m_dataCB)
			m_dataCB(m_handleType, m_user, &broadcastInfo);

		Response(rdata, PJSIP_SC_OK, NoHead);
		return true;
	}

	return false;
}
