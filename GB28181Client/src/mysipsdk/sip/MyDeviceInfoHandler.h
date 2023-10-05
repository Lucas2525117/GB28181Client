#ifndef _MY_DEVICEINFO_HANDLER_H_
#define _MY_DEVICEINFO_HANDLER_H_

#include "MyEventHandler.h"
#include "MyXmlParser.h"
#include "MyDynamicStruct.h"
#include "MySipContext.h"
#include "MySipInfo.h"

class CMyDeviceInfoHandler : public CMyEventHandler
{
public:
	CMyDeviceInfoHandler();
	virtual ~CMyDeviceInfoHandler();

	virtual bool OnReceive(pjsip_rx_data* rdata) override;
};

#endif

