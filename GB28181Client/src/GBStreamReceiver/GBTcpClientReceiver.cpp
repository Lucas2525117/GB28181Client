#include "GBTcpClientReceiver.h"

#define GB_UDP_RECV_BUF_SIZE (1024)

static void TcpDataCB(void* data, int len, void* userData)
{
	assert(userData);
	CGBTcpClientStreamReceiver* receiver = (CGBTcpClientStreamReceiver*)userData;
	receiver->InputTcpData(data, len);
}

static int TcpDataThread(void* param)
{
	assert(param);
	CGBTcpClientStreamReceiver* client = (CGBTcpClientStreamReceiver*)param;
	client->TcpDataWorker();
	return 0;
}

static void PSDataCB(void* data, INT32 len, void* userData)
{
	assert(userData);
	CGBTcpClientStreamReceiver* receiver = (CGBTcpClientStreamReceiver*)userData;
	receiver->InputPSData(data, len);
}

CGBTcpClientStreamReceiver::CGBTcpClientStreamReceiver(const char* gbUrl, StreamDataCallBack func, void* userParam)
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

	m_tcpClient = std::make_shared<ZDTcpClient>(TcpDataCB, this);
	if (!m_tcpClient.get() || 0 != m_tcpClient->TcpCreate())
		return -1;

	// ÉèÖÃ2s³¬Ê±
	int ret = m_tcpClient->TcpConnect(m_localIP.c_str(), m_localPort);
	if (0 != ret)
		return -1;

	if (0 != InitRtpSession_())
		return -1;

	m_thread = std::thread(TcpDataThread, this);
	return 0;
}

int CGBTcpClientStreamReceiver::Stop()
{
	if (m_tcpClient.get())
		m_tcpClient->TcpDestroy();

	if (m_rtpTcpTransmitter)
	{
		delete m_rtpTcpTransmitter;
		m_rtpTcpTransmitter = nullptr;
	}

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

int CGBTcpClientStreamReceiver::InitRtpSession_()
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

void CGBTcpClientStreamReceiver::TcpDataWorker()
{
	bool bAccept = false;
	uint8_t payload;
	while (m_running)
	{
		Poll();
		BeginDataAccess();

		if (GotoFirstSourceWithData())
		{
			do
			{
				RTPPacket* packet = nullptr;
				while (nullptr != (packet = GetNextPacket()))
				{
					payload = packet->GetPayloadType();
					if (0 == payload)
					{
						DeletePacket(packet);
						continue;
					}

					struct rtp_packet_tcp_active data;
					data.mark = packet->HasMarker();
					data.pts = packet->GetTimestamp();
					data.seq = packet->GetSequenceNumber();
					data.data = packet->GetPayloadData();
					data.len = (int)packet->GetPayloadLength();

					m_payload = payload;

					if (m_lastSeq < 0)
					{
						m_lastSeq = data.seq - 1;
					}

					if (m_lastSeq = data.seq - 1)
					{
						InputPSData(data.data, data.len);
					}

					DeletePacket(packet);
				}

			} while (GotoNextSourceWithData());
		}

		EndDataAccess();
		Sleep(30);
	}

	Destroy();
}