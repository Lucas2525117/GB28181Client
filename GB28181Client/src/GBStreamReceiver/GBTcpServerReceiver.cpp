#include "GBTcpServerReceiver.h"

#define GB_UDP_RECV_BUF_SIZE (1024)

static void TcpDataCB(void* data, int len, void* userData)
{
	assert(userData);
	CGBTcpServerStreamReceiver* receiver = (CGBTcpServerStreamReceiver*)userData;
	receiver->InputTcpData(data, len);
}

static void PSDataCB(void* data, INT32 len, void* userData)
{
	assert(userData);
	CGBTcpServerStreamReceiver* receiver = (CGBTcpServerStreamReceiver*)userData;
	receiver->InputPSData(data, len);
}

CGBTcpServerStreamReceiver::CGBTcpServerStreamReceiver(const char* gbUrl, GBDataCallBack func, void* userParam)
	: m_func(func)
	, m_userdata(userParam)
	, m_gbUrl(gbUrl)
	, m_rtp2PS(PSDataCB, this)
{
	ParseUrl_(gbUrl);
}

CGBTcpServerStreamReceiver::~CGBTcpServerStreamReceiver()
{
}

void CGBTcpServerStreamReceiver::DeleteThis()
{
	delete this;
}

int CGBTcpServerStreamReceiver::Start(int streamType)
{
	if (m_localIP.empty() || m_localPort <= 0)
		return -1;

	if (m_tcpServer.get())
		return 0;

	m_tcpServer = std::make_shared<TcpServer>(TcpDataCB, this);
	if (!m_tcpServer.get())
		return -1;

	if (0 != m_tcpServer->TcpCreate() 
		|| 0 != m_tcpServer->TcpBind(m_localPort) 
		|| 0 != m_tcpServer->TcpListen(5))
		return -1;

	return 0;
}

int CGBTcpServerStreamReceiver::Stop()
{
	if (m_tcpServer.get())
		m_tcpServer->TcpDestroy();

	return 0;
}

void CGBTcpServerStreamReceiver::InputTcpData(void* data, int len)
{
	m_rtp2PS.InputData(data, len);
}

void CGBTcpServerStreamReceiver::InputPSData(void* data, int len)
{
	if (!m_parse.get())
	{
		m_parse = std::make_shared<CPSParse>(m_func, m_userdata);
	}

	if (m_parse.get())
	{
		m_parse->InputData(data, len);
	}
}

int CGBTcpServerStreamReceiver::ParseUrl_(const std::string& gburl)
{
	size_t pos = gburl.find("gbtcps://");
	if (std::string::npos == pos)
		return -1;

	std::string ipp = gburl.substr(pos + 9, gburl.length());
	size_t pos_ipp = ipp.find(":");
	if (std::string::npos == pos_ipp)
		return -1;

	m_localIP = ipp.substr(0, pos_ipp);
	m_localPort = atoi(ipp.substr(pos_ipp + 1, ipp.length()).c_str());
	return 0;
}