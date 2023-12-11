#include "GBTcpServerSender.h"

int TCPData2PSThread(void* param)
{
	assert(param);
	CGBTcpServerSender* sender = (CGBTcpServerSender*)param;
	sender->TCPData2PSWorker();
	return 0;
}

static void PSTCPDataCB(int64_t pts, void* data, int len, void* userData)
{
	assert(userData);
	CGBTcpServerSender* sender = (CGBTcpServerSender*)userData;
	sender->InputPSTCPData(pts, data, len);
}

CGBTcpServerSender::CGBTcpServerSender(const char* gbUrl)
	: m_gbUrl(gbUrl)
{
	ParseUrl_(gbUrl);
}

CGBTcpServerSender::~CGBTcpServerSender()
{
	Stop();
}

void CGBTcpServerSender::DeleteThis()
{
	delete this;
}

int CGBTcpServerSender::Start()
{
	if (0 != m_localPort || m_tcpServer.get())
		return 0;

	int ret = -1;
	do 
	{
		m_tcpServer = std::make_shared<ZDTcpServer>(nullptr, this);
		if (!m_tcpServer.get())
			break;

		ret = m_tcpServer->TcpCreate();
		if (0 != ret)
			break;

		ret = m_tcpServer->TcpBind(m_localPort);
		if (0 != ret)
			break;

		ret = m_tcpServer->TcpListen(5);
		if (0 != ret)
			break;

		m_thread = std::thread(TCPData2PSThread, this);
		return 0;
	} while (0);

	Stop();
	return ret;
}

int CGBTcpServerSender::Stop()
{
	m_running = false;
	if (m_thread.joinable())
		m_thread.join();

	if (m_tcpServer.get())
		m_tcpServer->TcpDestroy();

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

int CGBTcpServerSender::SendData(void* data, int len)
{
	if (m_pspacker)
		m_pspacker->InputData(data, len);

	return 0;
}

int CGBTcpServerSender::GetStatus()
{
	return 0;
}

void CGBTcpServerSender::InputPSTCPData(int64_t pts, void* _data, int _len)
{
	SendPacket(_data, _len);
}

void CGBTcpServerSender::TCPData2PSWorker()
{
	if (!m_pspacker)
		m_pspacker = new(std::nothrow) CData2PS(PSTCPDataCB, this);

	bool bAccept = false;
	while (m_running)
	{
		if (!bAccept)
		{
			if (0 == m_tcpServer->TcpAccept())
			{
				bAccept = true;
				if (0 != InitRtpSession_())
				{
					break;
				}
			}

			continue;
		}

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

int CGBTcpServerSender::ParseUrl_(const std::string& gburl)
{
	size_t pos = gburl.find("gbudps://");
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

int CGBTcpServerSender::InitRtpSession_()
{
	const int packetSize = 45678;
	RTPSessionParams sessionparams;
	sessionparams.SetProbationType(RTPSources::NoProbation);
	//sessionparams.SetAcceptOwnPackets(true);
	sessionparams.SetOwnTimestampUnit(1.0 / packetSize);
	sessionparams.SetMaximumPacketSize(packetSize + 64);

	m_rtpTcpTransmitter = new RTPTCPTransmitter(nullptr);
	m_rtpTcpTransmitter->Init(true);
	m_rtpTcpTransmitter->Create(65535, 0);

	int status = Create(sessionparams, m_rtpTcpTransmitter);
	if (status < 0)
	{
		return status;
	}

	status = AddDestination(RTPTCPAddress(m_tcpServer->GetClientSocket()));
	if (0 != status)
		return status;

	SetDefaultPayloadType(96);
	SetDefaultMark(false);
	SetDefaultTimestampIncrement(160);
	return 0;
}