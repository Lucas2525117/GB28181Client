#include "MyByeHandler.h"

CMyByeHandler::CMyByeHandler()
{
}

CMyByeHandler::~CMyByeHandler()
{
}

bool CMyByeHandler::OnReceive(pjsip_rx_data* rdata)
{
	if (PJSIP_BYE_METHOD == rdata->msg_info.cseq->method.id)
	{
		Response(rdata, PJSIP_SC_OK, NoHead);
		return true;
	}

	return false;
}