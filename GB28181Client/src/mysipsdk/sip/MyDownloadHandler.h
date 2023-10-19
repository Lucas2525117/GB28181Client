#ifndef _MY_DOWNLOAD_HANDLER_H_
#define _MY_DOWNLOAD_HANDLER_H_

#include "MyEventHandler.h"
#include "MyXmlParser.h"
#include "MyDynamicStruct.h"
#include "MySipContext.h"
#include "MySipInfo.h"

class CMyDownloadHandler : public CMyEventHandler
{
public:
	CMyDownloadHandler();
	virtual ~CMyDownloadHandler();

	virtual bool OnReceive(pjsip_rx_data* rdata) override;
};

#endif

