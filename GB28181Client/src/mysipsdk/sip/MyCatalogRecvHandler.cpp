#include "MyCatalogRecvHandler.h"

CMyCatalogRecvHandler::CMyCatalogRecvHandler()
{

}

CMyCatalogRecvHandler::~CMyCatalogRecvHandler()
{
}

bool CMyCatalogRecvHandler::OnReceive(pjsip_rx_data* rdata)
{
	if (PJSIP_OTHER_METHOD == rdata->msg_info.cseq->method.id)
	{
		CMyXmlParser xmlParser(CMySipContext::GetInstance().GetMessageBody(rdata));
		CMyDynamicStruct dynamicStruct;
		dynamicStruct.Set(xmlParser.GetXml());

		auto cmd = xmlParser.GetXml()->RootElement()->Value();
		auto cmdType = dynamicStruct.Get("CmdType");
		if ("Catalog" != cmdType)
			return false;

		auto DeviceID = dynamicStruct.Get("DeviceID");
		std::string deviceList = dynamicStruct.Get("DeviceList");

		Json::Reader reader;
		Json::Value value; 
		reader.parse(deviceList, value);
		int size = value.size();
		for (int i = 0; i < size; i++)
		{
			Json::Value val = value[i];
			CMyCatalogInfo devinfo;
			devinfo.DeviceID = val["DeviceID"].asString();
			devinfo.Name = val["Name"].asString();  //??
			devinfo.Address = val["Address"].asString();
			devinfo.Manufacturer = val["Manufacturer"].asString();
			devinfo.Model = val["Model"].asString();
			devinfo.Owner = val["Owner"].asString();
			devinfo.Civilcode = val["CivilCode"].asString();
			devinfo.Registerway = val["RegisterWay"].asString();
			devinfo.Secrecy = val["Secrecy"].asString();
			//devinfo.IPAddress = val["IPAddress"].asString();
			devinfo.PlatformAddr = rdata->pkt_info.src_name;
			devinfo.PlatformPort = rdata->pkt_info.src_port;
			devinfo.Status = val["Status"].asString();

			if (m_dataCB)
				m_dataCB(m_handleType, m_user, &devinfo);
		}

		Response(rdata, PJSIP_SC_OK, NoHead);
		return true;
	}

	return false;
}

