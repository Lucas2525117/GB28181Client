#include "RtspTcpData.h"

static int RtpPacketCallback (void* param, const void* packet, int bytes, uint32_t timestamp, int flags)
{
	assert(param);
	CRtspTcpData* receiver = (CRtspTcpData*)param;
	receiver->Package(packet, bytes, timestamp, flags);
	return 0;
}

CRtspTcpData::CRtspTcpData(bool isVideo, StreamDataCallBack func, void* userParam)
	: m_isVideo(isVideo)
	, m_func(func)
	, m_user(userParam)
{
}

CRtspTcpData::~CRtspTcpData()
{
	if (m_demuxer)
	{
		rtp_demuxer_destroy(&m_demuxer);
		m_demuxer = nullptr;
	}
}

int CRtspTcpData::Start(int interleave1, int interleave2, int payload, const std::string& encoding)
{
	const struct rtp_profile_t* profile = rtp_profile_find(payload);
	m_demuxer = rtp_demuxer_create(100, profile ? profile->frequency : 90000, payload, encoding.c_str(), RtpPacketCallback, this);
	if (!m_demuxer)
		return -1;

	if (0 != CreatePacker_(payload, encoding))
		return -1;

	return 0;
}

bool CRtspTcpData::Stop()
{
	return true;
}

void CRtspTcpData::InputRtpData(const void* data, int len)
{
	if (!m_demuxer)
		return;
	
	rtp_demuxer_input(m_demuxer, data, len);
}

int CRtspTcpData::Package(const void* packet, int bytes, uint32_t timestamp, int flags)
{
	if (RTP_PAYLOAD_FLAG_PACKET_LOST == flags && 0 == bytes)
		return -1;

	if (m_xxx2Inpack.get())
		m_xxx2Inpack->InputData((void*)packet, bytes, timestamp);

	return 0;
}

int CRtspTcpData::CreatePacker_(int payload, const std::string& encoding)
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