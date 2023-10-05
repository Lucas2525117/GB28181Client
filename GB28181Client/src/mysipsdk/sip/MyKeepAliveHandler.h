#ifndef _MY_KEEPALIVE_HANDLER_H_
#define _MY_KEEPALIVE_HANDLER_H_

#include "MyEventHandler.h"
#include "MySipContext.h"
#include "MyXmlParser.h"
#include "MyDynamicStruct.h"
#include <string>

class CMyKeepAliveHandler : public CMyEventHandler
{
public:
	CMyKeepAliveHandler() {}

	virtual bool OnReceive(pjsip_rx_data* rdata) override;
};

#endif

