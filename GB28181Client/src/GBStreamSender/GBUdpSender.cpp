#include "GBUdpSender.h"

#include "rtpsession.h"
#include "rtpsessionparams.h"
#include "rtpudpv4transmitter.h"
#include "rtppacket.h"
#include "rtpsources.h"
#include "rtpsourcedata.h"
#include "rtpaddress.h"

#define RTP_MAX_PACKET_LEN   (1450)
#define RTP_PACKET_LEN       (1420)

int Data2PSThread(void* param)
{
	assert(param);
	CGBUdpSender* receiver = (CGBUdpSender*)param;
	receiver->Data2PSWorker();
	return 0;
}

static void PSDataCB(int64_t pts, void* data, int len, void* userData)
{
	assert(userData);
	CGBUdpSender* sender = (CGBUdpSender*)userData;
	sender->InputPSData(pts, data, len);
}

CGBUdpSender::CGBUdpSender(const char* gbUrl)
	: m_gbUrl(gbUrl)
{
	ParseUrl_(gbUrl);
}

CGBUdpSender::~CGBUdpSender()
{
	Stop();
}

void CGBUdpSender::DeleteThis()
{
	delete this;
}

int CGBUdpSender::Start()
{
	if (0 != m_localPort)
		return 0;

	m_status = InitRtpSession_();
	m_thread = std::thread(Data2PSThread, this);
	return m_status;
}

int CGBUdpSender::Stop()
{
	m_running = false;
	if (m_thread.joinable())
		m_thread.join();

	if (m_pspacker)
	{
		delete m_pspacker;
		m_pspacker = nullptr;
	}

	m_status = Status_Serve_Exiting;
	return 0;
}

int CGBUdpSender::SendData(void* data, int len)
{
	if (m_pspacker)
		m_pspacker->InputData(data, len);
	return 0;
}

int CGBUdpSender::GetStatus()
{
	return m_status;
}

void CGBUdpSender::InputPSData(int64_t pts, void* _data, int _len)
{
	char* data = (char*)_data;
	size_t bytes = _len;

	uint32_t incPts = (uint32_t)(pts - m_pts);
	if (0 == m_pts)
		incPts = 40;
	m_pts = pts;

	size_t len;
	bool mark;
	while (bytes > 0)
	{
		len = bytes > RTP_PACKET_LEN ? RTP_PACKET_LEN : bytes;
		bytes -= len;
		mark = (bytes <= 0);

		int ret = SendPacket(data, len, (uint8_t)(m_payload), mark, mark ? incPts * 90 : 0);
		if (0 != ret)
		{
		}

		data += len;
	}
}

void CGBUdpSender::Data2PSWorker()
{
	if (!m_pspacker)
		m_pspacker = new(std::nothrow) CData2PS(PSDataCB, this);

	while (m_running)
	{
		Poll();
		if (0 != m_lastRRTime && 0 == m_status)
		{
			auto now = std::chrono::system_clock::now();
			auto nowmillisecond = std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
			if (nowmillisecond - m_lastRRTime > 30 * 1000)
			{
				m_status = Status_NetWork_Broken;
			}
		}

		Sleep(10*1000);
	}
}

void CGBUdpSender::OnRTCPCompoundPacket(RTCPCompoundPacket* pack, const RTPTime& receivetime, const RTPAddress* senderaddress)
{
	pack->GotoFirstPacket();
	RTCPPacket* rtcpPacket = pack->GetNextPacket();
	while (rtcpPacket)
	{
		switch (rtcpPacket->GetPacketType())
		{
		case RTCPPacket::RR:
			m_lastRRTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
			break;
		case RTCPPacket::BYE:
			m_status = Status_NetWork_Broken;
			break;
		default:
			break;
		}

		rtcpPacket = pack->GetNextPacket();
	}
}

void CGBUdpSender::OnSendRTCPCompoundPacket(RTCPCompoundPacket* pack)
{
}

void CGBUdpSender::OnTimeout(RTPSourceData* srcdat)
{
	printf("rtcp timeout\n");
}

int CGBUdpSender::ParseUrl_(const std::string& gburl)
{
	size_t pos = gburl.find("gbudp://");
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

int CGBUdpSender::InitRtpSession_()
{
	RTPSessionParams sessionParams;
	sessionParams.SetMinimumRTCPTransmissionInterval(10);
	sessionParams.SetOwnTimestampUnit(1.0 / 90000.0);
	sessionParams.SetAcceptOwnPackets(true);
	sessionParams.SetMaximumPacketSize(RTP_MAX_PACKET_LEN);
	sessionParams.SetUsePredefinedSSRC(true);
	sessionParams.SetPredefinedSSRC(m_ssrc);

	uint32_t ip = inet_addr(m_localIP.c_str());
	ip = ntohl(ip);
	RTPUDPv4TransmissionParams transParams;
	transParams.SetRTPSendBuffer(4 * 1024 * 1024);
	transParams.SetBindIP(ip);
	transParams.SetPortbase((uint16_t)m_localPort);

	if (0 != Create(sessionParams, &transParams))
	{
		return -1;
	}

	SetDefaultPayloadType((uint8_t)m_payload);
	SetDefaultTimestampIncrement(3600);
	SetDefaultMark(true);

	RTPIPv4Address addr(ntohl(inet_addr(m_localIP .c_str())), (uint16_t)m_localPort);
	AddDestination(addr);

	return 0;
}