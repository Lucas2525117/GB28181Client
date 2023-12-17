#include "RtmpCommand.h"

static int SendCallback(void* param, const void* header, size_t len, const void* data, size_t bytes)
{
	assert(param);
	CRtmpCommand* command = (CRtmpCommand*)param;
	return command->Send(header, len, data, bytes);
}

static int AudioCallback(void* param, const void* data, size_t bytes, uint32_t timestamp)
{
	assert(param);
	CRtmpCommand* command = (CRtmpCommand*)param;
	return command->PlayAudio(data, bytes, timestamp);
}

static int VideoCallback(void* param, const void* data, size_t bytes, uint32_t timestamp)
{
	assert(param);
	CRtmpCommand* command = (CRtmpCommand*)param;
	return command->PlayVideo(data, bytes, timestamp);
}

static int ScriptCallback(void* param, const void* data, size_t bytes, uint32_t timestamp)
{
	assert(param);
	CRtmpCommand* command = (CRtmpCommand*)param;
	return command->PlayScript(data, bytes, timestamp);
}

static int RtmpDataCallbackFLV(void* param, int codec, const void* data, size_t bytes, uint32_t pts, uint32_t dts, int flags)
{
	assert(param);
	CRtmpCommand* command = (CRtmpCommand*)param;
	return command->Package(codec, data, bytes, pts, dts, flags);
}

CRtmpCommand::CRtmpCommand(ZDTcpClientPtr tcpClient, StreamDataCallBack func, void* userParam)
	: m_tcpClient(tcpClient)
	, m_func(func)
	, m_user(userParam)
{
}

CRtmpCommand::~CRtmpCommand()
{
}

int CRtmpCommand::Create(const std::string& appName, const std::string& playPath, const std::string& tcurl)
{
	if (appName.empty() || playPath.empty() || tcurl.empty())
		return -1;

	m_flvDemuxer = flv_demuxer_create(RtmpDataCallbackFLV, this);
	if (!m_flvDemuxer)
		return -1;

	rtmp_client_handler_t handler;
	memset(&handler, 0, sizeof(handler));
	handler.send = SendCallback;
	handler.onaudio = AudioCallback;
	handler.onvideo = VideoCallback;
	handler.onscript = ScriptCallback;
	m_rtmp = rtmp_client_create(appName.c_str(), playPath.c_str(), tcurl.c_str(), this, &handler);
	if (!m_rtmp)
		return -1;

	return 0;
}

void CRtmpCommand::Destroy()
{
	if (m_rtmp)
	{
		rtmp_client_stop(m_rtmp);
		rtmp_client_destroy(m_rtmp);
		m_rtmp = nullptr;
	}

	if (m_flvDemuxer)
	{
		flv_demuxer_destroy(m_flvDemuxer);
		m_flvDemuxer = nullptr;
	}
}

int CRtmpCommand::Start(int publish)
{
	if (!m_rtmp)
		return -1;

	return rtmp_client_start(m_rtmp, publish);
}

int CRtmpCommand::InputData(void* data, int len)
{
	if (!m_rtmp ||!data || len <= 0)
		return -1;

	return rtmp_client_input(m_rtmp, data, len);
}

int CRtmpCommand::Send(const void* header, int headerLen, const void* body, int bodyLen)
{
	int ret = -1;
	if (header && headerLen > 0)
	{
		ret = m_tcpClient->TcpSend((void*)header, headerLen, 0, 2000);
	}

	if (ret > 0 && body && bodyLen > 0)
	{
		ret = m_tcpClient->TcpSend((void*)body, bodyLen, 0, 2000);
	}

	return ret;
}

int CRtmpCommand::PlayVideo(const void* data, size_t bytes, uint32_t timestamp)
{
	if (!m_flvDemuxer || !data || bytes <= 0)
		return -1;

	return flv_demuxer_input(m_flvDemuxer, FLV_TYPE_VIDEO, data, bytes, timestamp);
}

int CRtmpCommand::PlayAudio(const void* data, size_t bytes, uint32_t timestamp)
{
	if (!m_flvDemuxer || !data || bytes <= 0)
		return -1;

	return flv_demuxer_input(m_flvDemuxer, FLV_TYPE_AUDIO, data, bytes, timestamp);
}

int CRtmpCommand::PlayScript(const void* data, size_t bytes, uint32_t timestamp)
{
	if (!m_flvDemuxer || !data || bytes <= 0)
		return -1;

	return flv_demuxer_input(m_flvDemuxer, FLV_TYPE_SCRIPT, data, bytes, timestamp);
}

int CRtmpCommand::Package(int codec, const void* data, size_t bytes, uint32_t pts, uint32_t dts, int flags)
{
	switch (codec)
	{
	case FLV_AUDIO_AAC:
		break;
	case FLV_VIDEO_H264:
		{
			if (!m_xxx2Inpack.get())
			{
				m_xxx2Inpack = std::make_shared<H264Inpack>(m_func, m_user, CODEC_VIDEO_H264, false);
			}
		}
		break;
	case FLV_VIDEO_H265:
		{
			if (!m_xxx2Inpack.get())
			{
				m_xxx2Inpack = std::make_shared<H265Inpack>(m_func, m_user, CODEC_VIDEO_H265, false);
			}
		}
		break;
	case FLV_AUDIO_MP3:
		break;
	case FLV_AUDIO_ASC:
		break;
	case FLV_VIDEO_AVCC:
		break;
	case FLV_VIDEO_HVCC:
		break;
	default:
		break;
	}

	if(m_xxx2Inpack.get())
		m_xxx2Inpack->InputData((void*)data, bytes, pts);
	return 0;
}
