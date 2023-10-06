#include "MyRecordInfoHandler.h"

CMyRecordInfoHandler::CMyRecordInfoHandler()
{

}

CMyRecordInfoHandler::~CMyRecordInfoHandler()
{
}

bool CMyRecordInfoHandler::OnReceive(pjsip_rx_data* rdata)
{
	if (PJSIP_OTHER_METHOD == rdata->msg_info.cseq->method.id)
	{
		CMyXmlParser xmlParser(CMySipContext::GetInstance().GetMessageBody(rdata));
		CMyDynamicStruct dynamicStruct;
		dynamicStruct.Set(xmlParser.GetXml());

		auto cmd = xmlParser.GetXml()->RootElement()->Value();
		auto cmdType = dynamicStruct.Get("CmdType");
		if ("RecordInfo" != cmdType)
			return false;

		// callback

		Response(rdata, PJSIP_SC_OK, NoHead);
		return true;
	}

	return false;
}

