#include "MySipMedia.h"
#include <objbase.h>

static int threadProc(void* param)
{
	assert(param);
	CMySipMedia* media = (CMySipMedia*)param;
	media->HandleEventProc();
	return 0;
}

CMySipMedia::CMySipMedia()
{
}

CMySipMedia::~CMySipMedia()
{
}

bool CMySipMedia::Init(const std::string& concat, int loglevel)
{
	if (concat.empty() || loglevel < 0)
		return false;

	bool ret = false;
	if (nullptr == m_mainModule)
	{
		if (!CMySipContext::GetInstance().Init(concat, loglevel))
			return false;

		static struct pjsip_module module =
		{
			nullptr, nullptr,
			{ (char*)"MainModule", 10 },
			-1,
			PJSIP_MOD_PRIORITY_APPLICATION,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			&CMySipMedia::OnReceive,
			nullptr,
			nullptr,
			nullptr,
		};

		m_mainModule = &module;
		pjsip_inv_callback cb;
		pj_bzero(&cb, sizeof(cb));
		cb.on_state_changed = &OnStateChanged;
		cb.on_new_session = &OnNewSession;
		cb.on_tsx_state_changed = &OnTsxStateChanged;
		cb.on_rx_offer = &OnRxOffer;
		cb.on_rx_reinvite = &OnRxReinvite;
		cb.on_create_offer = &OnCreateOffer;
		cb.on_send_ack = &OnSendAck;

		if (!CMySipContext::GetInstance().RegisterCallback(&cb))
			return false;

		CMySipContext::GetInstance().InitModule();
		if (!CMySipContext::GetInstance().RegisterModule(m_mainModule))
			return false;

		CMySipModule::GetInstance().Init();
		ret = CMySipContext::GetInstance().CreateWorkThread(&threadProc, m_workthread, this, "proxy");
	}

	return ret;
}

bool CMySipMedia::UnInit()
{
	m_running = false;
	if (nullptr != m_workthread)
	{
		CMySipContext::GetInstance().DestroyThread(m_workthread);
		m_workthread = nullptr;
	}

	if (!m_mainModule)
		return false;

	if (!CMySipContext::GetInstance().UnRegisterModule(m_mainModule))
		return false;

	CMySipContext::GetInstance().UnInit();
	return true;
}

void CMySipMedia::HandleEventProc()
{
	pj_time_val delay = { 0, 10 };
	while (m_running)
	{
		CMySipContext::GetInstance().HandleEvent(&delay);

		// 1s
		CMySipContext::GetInstance().PJThreadSleep(50);
	}
}

bool CMySipMedia::Invite(const GB28181MediaContext& mediaContext, void** token)
{
	*token = nullptr;

	auto localUri = CMySipContext::GetInstance().GetConcat();
	auto localContact = CMySipContext::GetInstance().GetConcat();
	auto target = CMySipContext::GetInstance().StrToPjstr(mediaContext.GetRequestUrl());
	auto remoteUri = CMySipContext::GetInstance().StrToPjstr(mediaContext.GetRequestUrl());
	pjsip_dialog* dlg = nullptr;
	pj_thread_desc rtpdesc;
	pj_thread_t* thread = nullptr;
	if (!pj_thread_is_registered())
	{
		pj_thread_register(nullptr, rtpdesc, &thread);
	}

	pj_str_t lUri = CMySipContext::GetInstance().StrToPjstr(localUri);
	pj_str_t lContact = CMySipContext::GetInstance().StrToPjstr(localContact);
	if (PJ_SUCCESS != pjsip_dlg_create_uac(pjsip_ua_instance(), &lUri, &lContact, &remoteUri, &target, &dlg))
		return false;

	GUID guid;
	HRESULT result = CoCreateGuid(&guid);
	if(S_OK != result)
		return false;


	char buf[64] = { 0, };
	_snprintf(buf, sizeof(buf),
		"%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1], guid.Data4[2],
		guid.Data4[3], guid.Data4[4], guid.Data4[5],
		guid.Data4[6], guid.Data4[7]);

	auto mytoken = buf;
	auto mySipDialog = std::make_shared<MySipDialog>(dlg);
	mySipDialog->RegisterHandle(std::bind(&CMySipMedia::ReceiveOk, this, std::placeholders::_1));
	
	{
		RecursiveGuard mtx(m_recursive_mutex);
		m_dialogs.emplace(mytoken, mySipDialog);
	}

	std::string sdp;
	if (StreamRequiredType::StreamType_RealStream == mediaContext.GetStreamType())
	{
		sdp = CreateSDPForRealStream(mediaContext);
	}
	else if (StreamRequiredType::StreamType_Playback == mediaContext.GetStreamType())
	{
		sdp = CreateSDPForPlayback(mediaContext);
	}
	else if (StreamRequiredType::StreamType_Download == mediaContext.GetStreamType())
	{
		sdp = CreateSDPForDownload(mediaContext);
	}

	CMySipContext::GetInstance().Invite(dlg, mediaContext, sdp);
	*token = mytoken;
	return true;
}

bool CMySipMedia::Subscribe(const GBSubscribeContext& subContext, void** token)
{
	*token = nullptr;

	auto localUri = CMySipContext::GetInstance().GetConcat();
	auto localContact = CMySipContext::GetInstance().GetConcat();
	auto target = CMySipContext::GetInstance().StrToPjstr(subContext.GetRequestUrl());
	auto remoteUri = CMySipContext::GetInstance().StrToPjstr(subContext.GetRequestUrl());
	pjsip_dialog* dlg = nullptr;
	pj_thread_desc rtpdesc;
	pj_thread_t* thread = nullptr;
	if (!pj_thread_is_registered())
	{
		pj_thread_register(nullptr, rtpdesc, &thread);
	}

	pj_str_t lUri = CMySipContext::GetInstance().StrToPjstr(localUri);
	pj_str_t lContact = CMySipContext::GetInstance().StrToPjstr(localContact);
	if (PJ_SUCCESS != pjsip_dlg_create_uac(pjsip_ua_instance(), &lUri, &lContact, &remoteUri, &target, &dlg))
		return false;

	GUID guid;
	HRESULT result = CoCreateGuid(&guid);
	if (S_OK != result)
		return false;

	char buf[64] = { 0, };
	_snprintf(buf, sizeof(buf),
		"%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1], guid.Data4[2],
		guid.Data4[3], guid.Data4[4], guid.Data4[5],
		guid.Data4[6], guid.Data4[7]);

	auto mytoken = buf;
	auto mySipDialog = std::make_shared<MySipDialog>(dlg);
	mySipDialog->RegisterHandle(std::bind(&CMySipMedia::ReceiveOk, this, std::placeholders::_1));

	{	
		RecursiveGuard mtx(m_recursive_mutex);
		m_dialogs.emplace(mytoken, mySipDialog);
	}

	std::string eventName;
	std::string xmlText;
	if (SubscribeType::Alarm_Subscribe == subContext.GetSubscribeType())
	{
		eventName = "Alarm";
		xmlText = CreateAlarmXmlText_(eventName, subContext);
	}
	else if (SubscribeType::MobilePosition_Subscribe == subContext.GetSubscribeType())
	{
		eventName = "MobilePosition";
		xmlText = CreateMobilePositionXmlText_(eventName, subContext);
	}
	else if (SubscribeType::PTZPosition_Subscribe == subContext.GetSubscribeType())
	{
		eventName = "PTZPosition";
		xmlText = CreatePTZPositionXmlText_(eventName, subContext);
	}
	else if (SubscribeType::Catalog_Subscribe == subContext.GetSubscribeType())
	{
		eventName = "Catalog";
		xmlText = CreateCatalogXmlText_(eventName, subContext);
	}

	if (!CMySipContext::GetInstance().Subscribe(dlg, eventName, xmlText, subContext))
		return false;

	*token = mytoken;
	return true;
}

void CMySipMedia::ReceiveOk(pjsip_rx_data* rdata)
{
	if (PJSIP_BYE_METHOD == rdata->msg_info.cseq->method.id)
		return;

	const auto tsx = pjsip_rdata_get_tsx(rdata);
	if (tsx == nullptr 
		|| tsx->status_code == 100 
		|| tsx->status_code == 101 
		|| tsx->status_code == 180) 
		return;

	SendAck(rdata);
}

void CMySipMedia::SendAck(pjsip_rx_data* rdata)
{
	auto dlg = pjsip_rdata_get_dlg(rdata);
	auto inv = pjsip_dlg_get_inv_session(dlg);
	if (!inv) 
		return;

	for (auto& dialog : m_dialogs)
	{
		if (dialog.second->GetDialog() == dlg)
		{
			dialog.second->RegisterHandle(std::bind(&CMySipMedia::ConfirmReceiveOk, this, std::placeholders::_1));
		}
	}

	pjsip_tx_data* tdata = nullptr;
	pjsip_inv_create_ack(inv, rdata->msg_info.cseq->cseq, &tdata);
	tdata->msg->body = nullptr;
	pjsip_inv_send_msg(inv, tdata);
}

void CMySipMedia::ConfirmReceiveOk(pjsip_rx_data* rdata) const
{
	if (PJSIP_BYE_METHOD == rdata->msg_info.cseq->method.id)
		return;
	
	pjsip_rdata_get_tsx(rdata);
}

int CMySipMedia::QueryDeviceStatus(int type, const std::string& gbid)
{
	MyGBDevicePtr device = CMyGBDeviceManager::GetInstance().GetDeviceById(gbid);
	if (!device.get())
		return -1;

	if(Type_RecvCatalog == type)
		CMySipContext::GetInstance().QueryDeviceInfo(device.get(), gbid, "Catalog");
	else if(Type_RecvDeviceInfo == type)
		CMySipContext::GetInstance().QueryDeviceInfo(device.get(), gbid, "DeviceInfo");
	else if(Type_RecvDeviceStatus == type)
		CMySipContext::GetInstance().QueryDeviceInfo(device.get(), gbid, "DeviceStatus");
	
	return 0;
}

int CMySipMedia::QueryRecordInfo(const std::string& gbid, const GB28181MediaContext& mediaContext)
{
	MyGBDevicePtr device = CMyGBDeviceManager::GetInstance().GetDeviceById(gbid);
	if (!device.get())
		return -1;

	CMySipContext::GetInstance().QueryRecordInfo(device.get(), gbid, mediaContext.GetStartTime(), mediaContext.GetEndTime(), "RecordInfo");
	return 0;
}

void CMySipMedia::RegisterHandler(int handleType, DataCallback dataCB, void* user)
{
	CMySipModule::GetInstance().RegisterHandler(handleType, dataCB, user);
}

int CMySipMedia::PTZControl(const std::string& gbid, PTZControlType controlType, int paramValue)
{
	MyGBDevicePtr device = CMyGBDeviceManager::GetInstance().GetDeviceById(gbid);
	if (!device.get())
		return -1;

	return CMySipContext::GetInstance().PTZControl(device.get(), gbid, controlType, paramValue);
}

bool CMySipMedia::Bye(const std::string& token)
{
	pj_thread_desc rtpdesc;
	pj_thread_t* thread = nullptr;
	if (!pj_thread_is_registered())
	{
		pj_thread_register(nullptr, rtpdesc, &thread);
	}

	auto result = m_dialogs.find(token);
	if (result != m_dialogs.end())
	{
		auto dlg = result->second->GetDialog();
		auto inv = pjsip_dlg_get_inv_session(dlg);
		if (!inv) 
			return false;

		if (PJSIP_INV_STATE_DISCONNECTED == inv->state)
			return true;

		pjsip_tx_data* tdata;
		pjsip_inv_end_session(inv, 200, nullptr, &tdata);
		pjsip_inv_send_msg(inv, tdata);
		m_dialogs.erase(result);
		return true;
	}
	return false;
}

std::string CMySipMedia::CreateAlarmXmlText_(const std::string& eventName, const GBSubscribeContext& subContext)
{
	char szAlarmInfo[200] = { 0 };
	pj_ansi_snprintf(szAlarmInfo, 200,
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<Query>\n"
		"<CmdType>%s</CmdType>\n"
		"<SN>17430</SN>\n"
		"<DeviceID>%s</DeviceID>\n"
		"</Query>\n", eventName.c_str(), subContext.GetDeviceId().c_str()
	);

	return szAlarmInfo;
}

std::string CMySipMedia::CreateMobilePositionXmlText_(const std::string& eventName, const GBSubscribeContext& subContext)
{
	char szAlarmInfo[200] = { 0 };
	pj_ansi_snprintf(szAlarmInfo, 200,
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<Query>\n"
		"<CmdType>%s</CmdType>\n"
		"<SN>17430</SN>\n"
		"<DeviceID>%s</DeviceID>\n"
		"</Query>\n", eventName.c_str(), subContext.GetDeviceId().c_str()
	);

	return szAlarmInfo;
}

std::string CMySipMedia::CreatePTZPositionXmlText_(const std::string& eventName, const GBSubscribeContext& subContext)
{
	char szAlarmInfo[200] = { 0 };
	pj_ansi_snprintf(szAlarmInfo, 200,
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<Query>\n"
		"<CmdType>%s</CmdType>\n"
		"<SN>17430</SN>\n"
		"<DeviceID>%s</DeviceID>\n"
		"</Query>\n", eventName.c_str(), subContext.GetDeviceId().c_str()
	);

	return szAlarmInfo;
}

std::string CMySipMedia::CreateCatalogXmlText_(const std::string& eventName, const GBSubscribeContext& subContext)
{
	char szAlarmInfo[200] = { 0 };
	pj_ansi_snprintf(szAlarmInfo, 200,
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<Query>\n"
		"<CmdType>%s</CmdType>\n"
		"<SN>17430</SN>\n"
		"<DeviceID>%s</DeviceID>\n"
		"</Query>\n", eventName.c_str(), subContext.GetDeviceId().c_str()
	);

	return szAlarmInfo;
}

