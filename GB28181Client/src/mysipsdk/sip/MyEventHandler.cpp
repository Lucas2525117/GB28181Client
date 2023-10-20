#include "MyEventHandler.h"

CMyEventHandler::CMyEventHandler()
{
}

CMyEventHandler::~CMyEventHandler()
{
}

void CMyEventHandler::RegisterCallback(int type, DataCallback dataCB, void* user)
{
	m_handleType = type;
	m_dataCB = dataCB;
	m_user = user;
}

void CMyEventHandler::Response(pjsip_rx_data* rdata, int st_code, int headType, const std::string& text)
{
	CMySipContext::GetInstance().Response(rdata, st_code, headType, text);
}