#ifndef _MY_EVENT_HANDLER_H_
#define _MY_EVENT_HANDLER_H_

#include "MySipContext.h"
#include "SipSDKPublic.h"

class CMyEventHandler
{
public:
	CMyEventHandler();
	~CMyEventHandler();

	virtual bool OnReceive(pjsip_rx_data* rdata) = 0;

	void RegisterCallback(int type, DataCallback dataCB, void* user);

protected:
	void Response(pjsip_rx_data* rdata, int st_code, int headType, const std::string& text = "", int subType = SUBTYPE_XML);

	int             m_handleType   = 0;
	DataCallback    m_dataCB = nullptr;
	void*           m_user   = nullptr;
};

typedef std::shared_ptr<CMyEventHandler> MyEventHandlerPtr;

#endif

