#include "RtspUdpData.h"

static int RtpPacketCB(void* param, const void* packet, int bytes, uint32_t timestamp, int flags)
{
	assert(param);
	CRtspUdpData* receiver = (CRtspUdpData*)param;
	receiver->Package(packet, bytes, timestamp, flags);
	return 0;
}

int UdpDataThread(void* param)
{
	assert(param);
	CRtspUdpData* receiver = (CRtspUdpData*)param;
	receiver->UdpDataWorker();
	return 0;
}

CRtspUdpData::CRtspUdpData(bool isVideo, StreamDataCallBack func, void* userParam)
	: m_isVideo(isVideo)
	, m_func(func)
	, m_user(userParam)
{
}

CRtspUdpData::~CRtspUdpData()
{
	Stop();
}

int CRtspUdpData::Start(int count, socket_t rtp[2], const char* peer, int peerport[2], int payload, const char* encoding)
{
	const struct rtp_profile_t* profile = rtp_profile_find(payload);
	m_demuxer = rtp_demuxer_create(100, profile ? profile->frequency : 90000, payload, encoding, RtpPacketCB, this);
	if (!m_demuxer)
		return -1;

	if ((0 != socket_addr_from(&m_sockaddrStorage[0], NULL, peer, (u_short)peerport[0]))
		|| (0 != socket_addr_from(&m_sockaddrStorage[1], NULL, peer, (u_short)peerport[1])))
	{
		rtp_demuxer_destroy(&m_demuxer);
		return -1;
	}

	if (0 != CreatePacker_(payload, encoding))
	{
		rtp_demuxer_destroy(&m_demuxer);
		return -1;
	}

	m_encoding = encoding;
	m_payload = payload;
	m_socket[0] = rtp[0];
	m_socket[1] = rtp[1];
	m_thread = std::thread(UdpDataThread, this);
	return 0;
}

void CRtspUdpData::Stop()
{
	m_running = false;
	if (m_thread.joinable())
		m_thread.join();

	if (m_demuxer)
	{
		rtp_demuxer_destroy(&m_demuxer);
		m_demuxer = nullptr;
	}

	if (m_xxx2Inpack.get())
	{
		m_xxx2Inpack.reset();
	}
}

int CRtspUdpData::CreatePacker_(int payload, const std::string& encoding)
{
	switch (payload)
	{
	case PayloadType_0:
		break;
	case PayloadType_8:
	case PayloadType_104:
		break;
	case PayloadType_96:
	case PayloadType_98:
	case PayloadType_105:
	{
		if (0 == strcmp("H264", encoding.c_str()))
			m_xxx2Inpack = std::make_shared<H264Inpack>(m_func, m_user, CODEC_VIDEO_H264, false);
		else if (0 == strcmp("H265", encoding.c_str()))
			m_xxx2Inpack = std::make_shared<H265Inpack>(m_func, m_user, CODEC_VIDEO_H265, false);
	}
	break;
	default:
		break;
	}

	return 0;
}

int CRtspUdpData::RtpRead_(socket_t s)
{
	if (!m_demuxer)
		return -1;

	struct sockaddr_storage ss;
	socklen_t len = sizeof(ss);
	int dataLen = recvfrom(s, m_rtpBuffer, sizeof(m_rtpBuffer), 0, (struct sockaddr*)&ss, &len);
	if (dataLen < 12)
	{
		return -1;
	}

	int ret = socket_addr_compare((const struct sockaddr*)&ss, (const struct sockaddr*)&m_sockaddrStorage[0]);
	if (0 != ret)
	{
		return -1;
	}

	return rtp_demuxer_input(m_demuxer, m_rtpBuffer, dataLen);
}

int CRtspUdpData::RtcpRead_(socket_t s)
{
	if (!m_demuxer)
		return -1;

	struct sockaddr_storage ss;
	socklen_t len = sizeof(ss);
	int dataLen = recvfrom(s, m_rtcpBuffer, sizeof(m_rtcpBuffer), 0, (struct sockaddr*)&ss, &len);
	if (dataLen < 12)
	{
		return -1;
	}

	assert(0 == socket_addr_compare((const struct sockaddr*)&ss, (const struct sockaddr*)&context->ss[1]));
	return rtp_demuxer_input(m_demuxer, m_rtcpBuffer, dataLen);
}

void CRtspUdpData::UdpDataWorker()
{
	int r = 0;
	int timeout = 1000;
	struct pollfd fds[2];

	for (int i = 0; i < 2; ++i)
	{
		fds[i].fd = m_socket[i];
		fds[i].events = POLLIN;
		fds[i].revents = 0;
	}

	while (m_running)
	{
		r = rtp_demuxer_rtcp(m_demuxer, m_rtcpBuffer, sizeof(m_rtcpBuffer));
		if (r > 0)
		{
			r = socket_sendto(m_socket[1], m_rtcpBuffer,
				r,
				0,
				(const struct sockaddr*)&m_sockaddrStorage[1],
				socket_addr_len((const struct sockaddr*)&m_sockaddrStorage[1]));
		}
			
		r = poll(fds, 2, timeout);
		while (-1 == r && EINTR == errno)
			r = poll(fds, 2, timeout);

		if (0 == r)
		{
			continue; // timeout
		}
		else if (r < 0)
		{
			return; // error
		}
		else
		{
			if (0 != fds[0].revents)
			{
				RtpRead_(m_socket[0]);
				fds[0].revents = 0;
			}

			if (0 != fds[1].revents)
			{
				RtcpRead_(m_socket[1]);
				fds[1].revents = 0;
			}
		}
	}
}

void CRtspUdpData::Package(const void* packet, int bytes, unsigned int timestamp, int flags)
{
	if (RTP_PAYLOAD_FLAG_PACKET_LOST == flags && 0 == bytes)
		return;

	if (m_xxx2Inpack.get())
	{
		m_xxx2Inpack->InputData((void*)packet, bytes, timestamp);
	}
}
