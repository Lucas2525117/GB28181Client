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

		std::string recordList = dynamicStruct.Get("RecordList");

		Json::Reader reader;
		Json::Value value;
		reader.parse(recordList, value);
		int size = value.size();
		for (int i = 0; i < size; i++)
		{
			Json::Value val = value[i];
			CMyRecordInfo recordinfo;
			recordinfo.deviceID = val["DeviceID"].asString();
			recordinfo.deviceName = val["Name"].asString();  
			recordinfo.filePath = val["FilePath"].asString();
			recordinfo.address = val["Address"].asString();
			recordinfo.startTime = val["StartTime"].asString();
			recordinfo.endTime = val["EndTime"].asString();
			recordinfo.fileSize = val["FileSize"].asString();
			recordinfo.secrecy = val["Secrecy"].asString();
			recordinfo.type = val["Type"].asString();

			if (m_dataCB)
				m_dataCB(m_handleType, m_user, &recordinfo);
		}

		Response(rdata, PJSIP_SC_OK, NoHead);
		return true;
	}

	return false;
}

