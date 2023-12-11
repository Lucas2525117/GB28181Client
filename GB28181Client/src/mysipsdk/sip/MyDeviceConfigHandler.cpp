#include "MyDeviceConfigHandler.h"

CMyDeviceConfigHandler::CMyDeviceConfigHandler()
{
}

CMyDeviceConfigHandler::~CMyDeviceConfigHandler()
{
}

bool CMyDeviceConfigHandler::OnReceive(pjsip_rx_data* rdata)
{
	if (PJSIP_OTHER_METHOD == rdata->msg_info.cseq->method.id)
	{
		CMyXmlParser xmlParser(CMySipContext::GetInstance().GetMessageBody(rdata));
		CMyDynamicStruct dynamicStruct;
		dynamicStruct.Set(xmlParser.GetXml());

		auto cmd = xmlParser.GetXml()->RootElement()->Value();
		auto cmdType = dynamicStruct.Get("CmdType");
		if ("UploadSnapShotFinished" == cmdType)
		{
			CMySnopShotFinishInfo snapShotInfo;
			snapShotInfo.deviceID = dynamicStruct.Get("DeviceID");
			snapShotInfo.sessionID = dynamicStruct.Get("SessionID");
			std::string snapShotList = dynamicStruct.Get("SnapShotList");

			Json::Reader reader;
			Json::Value value;
			reader.parse(snapShotList, value);
			int size = value.size();
			for (int i = 0; i < size; i++)
			{
				Json::Value val = value[i];
				std::string fileID = val["SnapShotFileID"].asString();
				snapShotInfo.fileIDs.push_back(fileID);
			}

			if (m_dataCB)
				m_dataCB(m_handleType, m_user, &snapShotInfo);
		}
		else
		{
			return false;
		}

		Response(rdata, PJSIP_SC_OK, NoHead);
		return true;
	}

	return false;
}