#ifndef _MY_RECORDINFO_HANDLER_H_
#define _MY_RECORDINFO_HANDLER_H_

#include "MyEventHandler.h"
#include "MyXmlParser.h"
#include "MyDynamicStruct.h"

class CMyRecordInfoHandler : public CMyEventHandler
{
public:
	CMyRecordInfoHandler();
	virtual ~CMyRecordInfoHandler();

	virtual bool OnReceive(pjsip_rx_data* rdata) override;
};

#endif

