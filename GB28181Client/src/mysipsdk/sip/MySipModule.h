#ifndef _MY_SIP_MODULE_H_
#define _MY_SIP_MODULE_H_

#include "MySipContext.h"
#include "MyEventHandler.h"
#include "SipSDKPublic.h"
#include "MySipHeader.h"
#include "MyRegisterHandler.h"
#include "MyKeepAliveHandler.h"
#include "MyCatalogRecvHandler.h"
#include "MyDeviceInfoHandler.h"
#include "MyDeviceStatusHandler.h"
#include "MyRecordInfoHandler.h"
#include "MyDownloadHandler.h"
#include "MyAlarmHandler.h"
#include <vector>
#include <memory>

class CMySipModule
{
public:
	static CMySipModule& GetInstance()
	{
		static CMySipModule sipModule;
		return sipModule;
	}

	bool Init();

	void RegisterHandler(int type, DataCallback dataCB, void* user);

	void OnSubNotify(pjsip_rx_data* rdata);

private:
	CMySipModule();
	~CMySipModule();

	static pj_bool_t OnReceive(pjsip_rx_data* rdata)
	{
		RecursiveGuard mtx(GetInstance().m_recursive_mutex);
		for (auto& handle : GetInstance().m_handlers)
		{
			if(handle)
				handle->OnReceive(rdata);
		}
		return PJ_TRUE;
	}

private:
	std::vector<MyEventHandlerPtr> m_handlers;
	pjsip_module* m_module;

	typedef std::lock_guard<std::recursive_mutex> RecursiveGuard;
	std::recursive_mutex m_recursive_mutex;
};

#endif

