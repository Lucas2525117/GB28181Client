#ifndef _MY_DEVICECONFIG_HANDLER_H_
#define _MY_DEVICECONFIG_HANDLER_H_

#include "MyEventHandler.h"
#include "MyXmlParser.h"
#include "MyDynamicStruct.h"
#include "MySipContext.h"
#include "MySipInfo.h"

class CMyDeviceConfigHandler : public CMyEventHandler
{
public:
	CMyDeviceConfigHandler();
	virtual ~CMyDeviceConfigHandler();

	virtual bool OnReceive(pjsip_rx_data* rdata) override;
};

#endif

