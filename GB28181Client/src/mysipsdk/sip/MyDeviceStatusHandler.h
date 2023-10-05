#ifndef _MY_DEVICESTATUS_HANDLER_H_
#define _MY_DEVICESTATUS_HANDLER_H_

#include "MyEventHandler.h"
#include "MyXmlParser.h"
#include "MyDynamicStruct.h"
#include "MySipContext.h"
#include "MySipInfo.h"

class CMyDeviceStatusHandler : public CMyEventHandler
{
public:
	CMyDeviceStatusHandler();
	virtual ~CMyDeviceStatusHandler();

	virtual bool OnReceive(pjsip_rx_data* rdata) override;
};

#endif

