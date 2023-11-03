#include "GBTcpClientReceiver.h"

#define GB_UDP_RECV_BUF_SIZE (1024)

static void TcpDataCB(void* data, int len, void* userData)
{
	assert(userData);
	CGBTcpClientStreamReceiver* receiver = (CGBTcpClientStreamReceiver*)userData;
	receiver->InputTcpData(data, len);
}

static void PSDataCB(void* data, INT32 len, void* userData)
{
	assert(userData);
	CGBTcpClientStreamReceiver* receiver = (CGBTcpClientStreamReceiver*)userData;
	receiver->InputPSData(data, len);
}

CGBTcpClientStreamReceiver::CGBTcpClientStreamReceiver(const char* gbUrl, GBDataCallBack func, void* userParam)
	: m_func(func)
	, m_userdata(userParam)
	, m_gbUrl(gbUrl)
	, m_rtp2PS(PSDataCB, this)
{
	ParseUrl_(gbUrl);
}

CGBTcpClientStreamReceiver::~CGBTcpClientStreamReceiver()
{
}

void CGBTcpClientStreamReceiver::DeleteThis()
{
	delete this;
}

int CGBTcpClientStreamReceiver::Start(int streamType)
{
	if (m_localIP.empty() || m_localPort <= 0)
		return -1;

	if (m_tcpClient.get())
		return 0;

	m_tcpClient = std::make_shared<TcpClient>(TcpDataCB, this);
	if (!m_tcpClient.get() || 0 != m_tcpClient->TcpCreate())
		return -1;

	// ÉèÖÃ2s³¬Ê±
	return m_tcpClient->TcpConnectByTime(m_localIP.c_str(), m_localPort, 2);
}

int CGBTcpClientStreamReceiver::Stop()
{
	return 0;
}

int CGBTcpClientStreamReceiver::SetCodec(int codec)
{
	return 0;
}

void CGBTcpClientStreamReceiver::InputTcpData(void* data, int len)
{
	m_rtp2PS.InputData(data, len);
}

void CGBTcpClientStreamReceiver::InputPSData(void* data, int len)
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

int CGBTcpClientStreamReceiver::ParseUrl_(const std::string& gburl)
{
	size_t pos = gburl.find("gbtcpc://");
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