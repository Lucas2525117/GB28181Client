#ifndef _MY_REGISTER_HANDLER_H_
#define _MY_REGISTER_HANDLER_H_

#include "MyGBDevice.h"
#include "MyEventHandler.h"
#include "MyGBDeviceManager.h"
#include <memory>
#include <winsock.h>

class CMyRegisterHandler : public CMyEventHandler
{
public:
	CMyRegisterHandler();
	virtual ~CMyRegisterHandler();

	virtual bool OnReceive(pjsip_rx_data* rdata);

	void OnWorker();

private:
	void QureryDeviceInfo(pjsip_rx_data* rdata);
	void UpdateDeviceStatus();

private:
	MyGBDevicePtr                m_device;
	pj_thread_t*                 m_queryStatusThread;
	int                          m_queryStatusInterval = 30*1000;
	bool                         m_hasDevice = false;
	bool                         m_running = true;
	std::string                  m_dstIp;
	int                          m_dstPort;
};

#endif

