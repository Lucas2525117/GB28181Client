#ifndef _MY_VOICE_BROADCAST_HANDLER_H_
#define _MY_VOICE_BROADCAST_HANDLER_H_

#include "MyEventHandler.h"
#include "MyXmlParser.h"
#include "MyDynamicStruct.h"
#include "MySipContext.h"
#include "MySipInfo.h"

class CMyVoiceBroadcastHandler : public CMyEventHandler
{
public:
	CMyVoiceBroadcastHandler();
	virtual ~CMyVoiceBroadcastHandler();

	virtual bool OnReceive(pjsip_rx_data* rdata) override;
};

#endif  // _MY_VOICE_BROADCAST_HANDLER_H_

