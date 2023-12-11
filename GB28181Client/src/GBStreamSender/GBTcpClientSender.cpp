#include "GBTcpClientSender.h"

int RTPPackerThread(void* param)
{
	assert(param);
	CGBTcpClientSender* sender = (CGBTcpClientSender*)param;
	sender->RTPPackWorker();
	return 0;
}

static void PSDataCB(int64_t pts, void* data, int len, void* userData)
{
	assert(userData);
	CGBTcpClientSender* sender = (CGBTcpClientSender*)userData;
	sender->InputPSTCPData(pts, data, len);
}

CGBTcpClientSender::CGBTcpClientSender(const char* gbUrl)
	: m_gbUrl(gbUrl)
{
	ParseUrl_(gbUrl);
}

CGBTcpClientSender::~CGBTcpClientSender()
{
	Stop();
}

void CGBTcpClientSender::DeleteThis()
{
	delete this;
}

int CGBTcpClientSender::Start()
{
	if (0 != m_localPort || m_tcpClient.get())
		return 0;

	int ret = -1;
	do
	{
		m_tcpClient = std::make_shared<ZDTcpClient>(nullptr, this);
		if (!m_tcpClient.get() || 0 != m_tcpClient->TcpCreate())
			break;

		ret = m_tcpClient->TcpConnect(m_localIP.c_str(), m_localPort);
		if (0 != ret)
			break;

		ret = InitRtpSession_();
		if (0 != ret)
			break;

		m_thread = std::thread(RTPPackerThread, this);
		return 0;
	} while (0);

	Stop();
	return ret;
}

int CGBTcpClientSender::Stop()
{
	m_running = false;
	if (m_thread.joinable())
		m_thread.join();

	if (m_tcpClient.get())
		m_tcpClient->TcpDestroy();

	if (m_pspacker)
	{
		delete m_pspacker;
		m_pspacker = nullptr;
	}

	if (m_rtpTcpTransmitter)
	{
		delete m_rtpTcpTransmitter;
		m_rtpTcpTransmitter = nullptr;
	}

	return 0;
}

int CGBTcpClientSender::SendData(void* data, int len)
{
	if (m_pspacker)
		m_pspacker->InputData(data, len);

	return 0;
}

int CGBTcpClientSender::GetStatus()
{
	return 0;
}

void CGBTcpClientSender::InputPSTCPData(int64_t pts, void* _data, int _len)
{
	SendPacket(_data, _len);
}

void CGBTcpClientSender::RTPPackWorker()
{
	if (!m_pspacker)
		m_pspacker = new(std::nothrow) CData2PS(PSDataCB, this);

	while (m_running)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

int CGBTcpClientSender::ParseUrl_(const std::string& gburl)
{
	size_t pos = gburl.find("gbudpc://");
	if (std::string::npos == pos)
		return -1;

	std::string ipp = gburl.substr(pos + 8, gburl.length());
	size_t pos_ipp = ipp.find(":");
	if (std::string::npos == pos_ipp)
		return -1;

	m_localIP = ipp.substr(0, pos_ipp);
	m_localPort = atoi(ipp.substr(pos_ipp + 1, ipp.length()).c_str());
	return 0;
}

int CGBTcpClientSender::InitRtpSession_()
{
	const int packSize = 45678;
	RTPSessionParams sessionParams;
	sessionParams.SetProbationType(RTPSources::NoProbation);
	sessionParams.SetOwnTimestampUnit(90000.0 / 25.0);
	sessionParams.SetMaximumPacketSize(packSize + 64);

	m_rtpTcpTransmitter = new RTPTCPTransmitter(nullptr);
	m_rtpTcpTransmitter->Init(true);
	m_rtpTcpTransmitter->Create(65535, 0);

	if (0 != Create(sessionParams, m_rtpTcpTransmitter))
		return -1;

	if (0 != AddDestination(RTPTCPAddress(m_tcpClient->GetClientSocket())))
		return -1;

	return 0;
}
