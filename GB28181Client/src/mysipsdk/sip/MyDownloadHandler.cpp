#include "MyDownloadHandler.h"

CMyDownloadHandler::CMyDownloadHandler()
{
}

CMyDownloadHandler::~CMyDownloadHandler()
{
}

bool CMyDownloadHandler::OnReceive(pjsip_rx_data* rdata)
{
	if (PJSIP_OTHER_METHOD == rdata->msg_info.cseq->method.id)
	{
		CMyXmlParser xmlParser(CMySipContext::GetInstance().GetMessageBody(rdata));
		CMyDynamicStruct dynamicStruct;
		dynamicStruct.Set(xmlParser.GetXml());

		auto cmd = xmlParser.GetXml()->RootElement()->Value();
		auto cmdType = dynamicStruct.Get("CmdType");
		if ("MediaStatus" != cmdType)
			return false;

		CMyDownloadInfo downloadInfo;
		downloadInfo.deviceID = dynamicStruct.Get("DeviceID");
		downloadInfo.notifyType = dynamicStruct.Get("NotifyType");

		if (m_dataCB)
			m_dataCB(m_handleType, m_user, &downloadInfo);

		Response(rdata, PJSIP_SC_OK, NoHead);
		return true;
	}

	return false;
}