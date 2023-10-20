#ifndef _MY_ALARM_HANDLER_H_
#define _MY_ALARM_HANDLER_H_

#include "MyEventHandler.h"
#include "MyXmlParser.h"
#include "MyDynamicStruct.h"
#include "MySipContext.h"
#include "MySipInfo.h"

class CMyAlarmHandler : public CMyEventHandler
{
public:
	CMyAlarmHandler();
	virtual ~CMyAlarmHandler();

	virtual bool OnReceive(pjsip_rx_data* rdata) override;
};

#endif

