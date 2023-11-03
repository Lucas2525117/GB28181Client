#ifndef _MY_BYE_HANDLER_H_
#define _MY_BYE_HANDLER_H_

#include "MyEventHandler.h"
#include "MyXmlParser.h"
#include "MyDynamicStruct.h"
#include "MySipContext.h"
#include "MySipInfo.h"

class CMyByeHandler : public CMyEventHandler
{
public:
	CMyByeHandler();
	virtual ~CMyByeHandler();

	virtual bool OnReceive(pjsip_rx_data* rdata) override;
};

#endif

