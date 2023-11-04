#include "MySipModule.h"

CMySipModule::CMySipModule()
{
}

CMySipModule::~CMySipModule()
{
}

bool CMySipModule::Init()
{
	static struct pjsip_module m =
	{
		nullptr, nullptr,
		{ (char*)"sipcontrolmodule", 16 },
		-1,
		PJSIP_MOD_PRIORITY_APPLICATION,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		&CMySipModule::OnReceive,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
	};

	m_module = &m;
	return CMySipContext::GetInstance().RegisterModule(m_module);
}

void CMySipModule::RegisterHandler(int type, DataCallback dataCB, void* user)
{
	std::shared_ptr<CMyEventHandler> handle = nullptr;
	switch (type)
	{
	case Type_Register:
		handle = std::make_shared<CMyRegisterHandler>();
		break;
	case Type_KeepAlive:
		handle = std::make_shared<CMyKeepAliveHandler>();
		break;
	case Type_RecvCatalog:
		handle = std::make_shared<CMyCatalogRecvHandler>();
		break;
	case Type_RecvDeviceInfo:
		handle = std::make_shared<CMyDeviceInfoHandler>();
		break;
	case Type_RecvDeviceStatus:
		handle = std::make_shared<CMyDeviceStatusHandler>();
		break;
	case Type_RecvRecordInfo:
		handle = std::make_shared<CMyRecordInfoHandler>();
		break;
	case Type_Download:
		handle = std::make_shared<CMyDownloadHandler>();
		break;
	case Type_Alarm:
		handle = std::make_shared<CMyAlarmHandler>();
		break;
	case Type_VoiceBroadcast:
		handle = std::make_shared<CMyVoiceBroadcastHandler>();
		break;
	case Type_Invite:
	case Type_VideoInvite:
		handle = std::make_shared<CMyInviteHandler>();
		break;
	case Type_Bye:
		handle = std::make_shared<CMyByeHandler>();
		break;
	default:
		break;
	}

	if (handle)
	{
		handle->RegisterCallback(type, dataCB, user);
		{
			RecursiveGuard mtx(rmutex_);
			m_handlers.push_back(handle);
		}
	}
}

void CMySipModule::OnSubNotify(pjsip_rx_data* rdata)
{
	OnReceive(rdata);
}