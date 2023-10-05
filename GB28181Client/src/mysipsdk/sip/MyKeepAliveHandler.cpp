#include "MyKeepAliveHandler.h"

bool CMyKeepAliveHandler::OnReceive(pjsip_rx_data* rdata)
{
	if (PJSIP_OTHER_METHOD == rdata->msg_info.cseq->method.id)
	{
		CMyXmlParser xmlParser(CMySipContext::GetInstance().GetMessageBody(rdata));
		CMyDynamicStruct dynamicStruct;
		dynamicStruct.Set(xmlParser.GetXml());
		std::string cmd = xmlParser.GetXml()->RootElement()->Value();
		std::string cmdType = dynamicStruct.Get("CmdType");
		if ("Notify" != cmd || "Keepalive" != cmdType) 
			return false;

		CMySipContext::GetInstance().Response(rdata, PJSIP_SC_OK, NoHead);
	}

	return true;
}