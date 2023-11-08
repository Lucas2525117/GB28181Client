#include "GBTcpServerReceiver.h"
#include "rtpsessionparams.h"
#include "rtptcptransmitter.h"
#include "rtptcpaddress.h"
#include "rtppacket.h"
#include "rtpsources.h"
#include "rtpsourcedata.h"
#include "rtpaddress.h"
#include "rtpipv4address.h"

static void TcpDataCB(void* data, int len, void* userData)
{
	assert(userData);
	CGBTcpServerStreamReceiver* receiver = (CGBTcpServerStreamReceiver*)userData;
	receiver->InputTcpData(data, len);
}

static int TcpDataThread(void* param)
{
	assert(param);
	CGBTcpServerStreamReceiver* client = (CGBTcpServerStreamReceiver*)param;
	client->TcpDataWorker();
	return 0;
}

static void PSDataCB(void* data, INT32 len, void* userData)
{
	assert(param);
	CGBTcpServerStreamReceiver* client = (CGBTcpServerStreamReceiver*)userData;
	client->InputRtpData(data, len);
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

	//m_thread = std::thread(TcpDataThread, this);
	return 0;
}

int CGBTcpServerStreamReceiver::Stop()
{
	m_running = false;
	if (m_thread.joinable())
		m_thread.join();

	if (m_tcpServer.get())
		m_tcpServer->TcpDestroy();

	return 0;
}

void CGBTcpServerStreamReceiver::InputTcpData(void* data, int len)
{
	m_rtp2PS.InputData(data, len);
}

void CGBTcpServerStreamReceiver::InputRtpData(void* data, int len)
{
	PackData_(data, len);
}

void CGBTcpServerStreamReceiver::PackData_(void* data, int len)
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

int CGBTcpServerStreamReceiver::InitRtpSession_()
{
	const int packSize = 1500;
	RTPSessionParams sessionParams;
	sessionParams.SetProbationType(RTPSources::NoProbation);
	sessionParams.SetOwnTimestampUnit(1.0 / packSize);
	sessionParams.SetMaximumPacketSize(packSize + 64);

	RTPTCPTransmitter transmitter(0);
	transmitter.Init(true);
	transmitter.Create(65535, 0);

	if (0 != Create(sessionParams, &transmitter))
		return -1;

	if (0 != AddDestination(RTPTCPAddress(m_tcpServer->GetClientSocket())))
		return -1;

	return 0;
}

void CGBTcpServerStreamReceiver::TcpDataWorker()
{
	bool bAccept = false;
	uint8_t payload;
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

					struct rtp_packet_tcp data;
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
						PackData_(data.data, data.len);
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