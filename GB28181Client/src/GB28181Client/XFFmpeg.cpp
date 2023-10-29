#include "XFFmpeg.h"
#include <map>

std::map<CodecType, AVCodecID> g_codecConvert;

static bool IsAudio(int avtype)
{
	switch (avtype)
	{
	case CODEC_AUDIO_G711:
	case CODEC_AUDIO_G711U:
	case CODEC_AUDIO_PCM:
	case CODEC_AUDIO_AAC:
		return true;
	default:
		return false;
	}

	return false;
}

static bool IsVideo(int avtype)
{
	switch (avtype)
	{
	case CODEC_VIDEO_MPEG4:
	case CODEC_VIDEO_H264:
	case CODEC_VIDEO_H265:
		return true;
	default:
		return false;
	}

	return false;
}

static int VideoDecoderThread(void* param)
{
	XFFmpeg* p = (XFFmpeg*)param;
	if (p)
	{
		p->OnVideoWorkThread();
	}
	return 0;
}

XFFmpeg::XFFmpeg(VideoYuvCallBack pVideoFunc, AudioDataCallBack pAudioFunc, void* userData)
: m_pVideoFunc(pVideoFunc), m_pAudioFunc(pAudioFunc), m_userData(userData)
, m_pCodecCtxVideo(nullptr), m_pCodecVideo(nullptr), m_pFrameVideo(nullptr)
, m_pCodecCtxAudio(nullptr), m_pCodecAudio(nullptr)
, m_frameAudio(nullptr), m_audioPacket(nullptr)
, m_imgConvertCtx(nullptr), m_picture(nullptr), m_pictureBuf(nullptr)
, m_audioSwrCtx(nullptr), m_decodeAudioBuffer(nullptr), m_audioBuffer(nullptr)
, m_oneAudioFrameLength(0), m_decodeAudioLength(0)
, m_ySize(0), m_count(0), m_head(0), m_tail(0)
, m_running(true)
{
	memset(&m_frameBuf, 0, sizeof(YuvFrame)*YUV_BUFSIZE);

	g_codecConvert[CODEC_VIDEO_MPEG4] = AV_CODEC_ID_MPEG4;
	g_codecConvert[CODEC_VIDEO_H264] = AV_CODEC_ID_H264;
	g_codecConvert[CODEC_VIDEO_H265] = AV_CODEC_ID_H265;

	g_codecConvert[CODEC_AUDIO_G711] = AV_CODEC_ID_PCM_ALAW;
	g_codecConvert[CODEC_AUDIO_G711U] = AV_CODEC_ID_PCM_MULAW;
	g_codecConvert[CODEC_AUDIO_PCM] = AV_CODEC_ID_FIRST_AUDIO;
	g_codecConvert[CODEC_AUDIO_AAC] = AV_CODEC_ID_AAC;

	m_videoThread = std::thread(VideoDecoderThread, this);
}

XFFmpeg::~XFFmpeg()
{
	m_running = false;
	if (m_videoThread.joinable())
		m_videoThread.join();
	if (m_audioThread.joinable())
		m_audioThread.join();
	CloseVideo();
	CloseAudio();

	{
		RecursiveGuard mtx(rVideoMutex);
		m_count = 0;
		for (int i = 0; i < YUV_BUFSIZE; ++i)
		{
			if (m_frameBuf[i].y != NULL)
				free(m_frameBuf[i].y);
		}
	}
}

int XFFmpeg::CreateVideo(unsigned char codec)
{
	std::map<CodecType, AVCodecID>::iterator it = g_codecConvert.find((CodecType)codec);
	if (it == g_codecConvert.end())
	{
		printf("%s:%d| not support codec %d\n", __FUNCTION__, __LINE__, codec);
		return -1;
	}

	m_pCodecVideo = avcodec_find_decoder(it->second);
	if (m_pCodecVideo == NULL)
	{
		printf("%s:%d| avcodec_find_decoder failed\n", __FUNCTION__, __LINE__);
		return -1;
	}

	m_pCodecCtxVideo = avcodec_alloc_context3(m_pCodecVideo);
	if (!m_pCodecCtxVideo)
	{
		printf("%s:%d| avcodec_alloc_context3 error\n", __FUNCTION__, __LINE__);
		return -1;
	}

	if (avcodec_open2(m_pCodecCtxVideo, m_pCodecVideo, NULL) < 0)
	{
		printf("%s:%d| avcodec_open2 failed\n", __FUNCTION__, __LINE__);
		return -1;
	}

	m_pFrameVideo = av_frame_alloc(); 
	if (m_pFrameVideo == NULL)
	{
		printf("%s:%d| av_frame_alloc failed\n", __FUNCTION__, __LINE__);
		return -1;
	}
	
	return 0;
}

void XFFmpeg::CloseVideo()
{
	if (m_pCodecCtxVideo != NULL)
	{
		avcodec_close(m_pCodecCtxVideo);
		av_free(m_pCodecCtxVideo);
	}

	if (m_pFrameVideo != NULL)
	{
		av_frame_free(&m_pFrameVideo);
		m_pFrameVideo = NULL;
	}

	if (m_picture != NULL)
	{
		av_frame_free(&m_picture);
		m_picture = NULL;
	}

	if (m_pictureBuf != NULL)
	{
		delete[] m_pictureBuf;
		m_pictureBuf = NULL;
	}

	if (m_imgConvertCtx != NULL)
	{
		sws_freeContext(m_imgConvertCtx);
		m_imgConvertCtx = NULL;
	}
}

int XFFmpeg::CreateAudio(const char* fileName, unsigned char codec)
{
	std::map<CodecType, AVCodecID>::iterator it = g_codecConvert.find((CodecType)codec);
	if (it == g_codecConvert.end())
	{
		printf("%s:%d| not support codec %d\n", __FUNCTION__, __LINE__, codec);
		return -1;
	}

	// 获取音频解码器
	m_pCodecAudio = avcodec_find_decoder(it->second);
	if (nullptr == m_pCodecAudio)
	{
		printf("%s:%d| avcodec_find_decoder failed\n", __FUNCTION__, __LINE__);
		return -1;
	}

	// 打开音频解码器
	m_pCodecCtxAudio = avcodec_alloc_context3(m_pCodecAudio);
	if (!m_pCodecCtxAudio)
	{
		printf("%s:%d| avcodec_alloc_context3 error\n", __FUNCTION__, __LINE__);
		return -1;
	}

	int audioCodecType = (int)codec;
	switch (audioCodecType)
	{
	case CODEC_AUDIO_AAC:
		break;
	case CODEC_AUDIO_MP3:
		break;
	case CODEC_AUDIO_G711:
	case CODEC_AUDIO_G711U:
		m_pCodecCtxAudio->codec_type = AVMEDIA_TYPE_AUDIO;
		m_pCodecCtxAudio->sample_fmt = AV_SAMPLE_FMT_S16;
		m_pCodecCtxAudio->sample_rate = 8000;
		m_pCodecCtxAudio->channel_layout = AV_CH_LAYOUT_MONO;
		m_pCodecCtxAudio->channels = 1;
		break;
	case CODEC_AUDIO_G7231:
		break;
	case CODEC_AUDIO_G7221:
		break;
	default:
		break;
	}

	int ret = avcodec_open2(m_pCodecCtxAudio, m_pCodecAudio, nullptr);
	if (ret < 0)
	{
		printf("%s:%d| Failed to open codec\n", __FUNCTION__, __LINE__);
		return -1;
	}

	m_frameAudio = av_frame_alloc();
	if (nullptr == m_frameAudio)
	{
		printf("%s:%d| av_frame_alloc failed\n", __FUNCTION__, __LINE__);
		return -1;
	}

	m_audioPacket = av_packet_alloc();
	if (nullptr == m_audioPacket)
	{
		printf("%s:%d| av_packet_alloc failed\n", __FUNCTION__, __LINE__);
		return -1;
	}
	av_init_packet(m_audioPacket);

	m_decodeAudioBuffer = new unsigned char[MaxDecodeAudioBufferCache];
	m_audioBuffer = new unsigned char[MaxOneDecodeAudioLength];

	return 0;
}

void XFFmpeg::CloseAudio()
{
	// 关闭音频解码器和输入文件，释放缓冲区和FIFO
	if (nullptr != m_pCodecCtxAudio)
	{
		avcodec_close(m_pCodecCtxAudio);
		av_free(m_pCodecCtxAudio);
		m_pCodecCtxAudio = nullptr;
	}

	if (nullptr != m_frameAudio)
	{
		av_frame_free(&m_frameAudio);
		m_frameAudio = nullptr;
	}

	if (nullptr != m_audioPacket)
	{
		av_packet_unref(m_audioPacket);
		av_packet_free(&m_audioPacket);
		m_audioPacket = nullptr;
	}

	if (nullptr != m_audioSwrCtx)
	{
		swr_free(&m_audioSwrCtx);
		m_audioSwrCtx = nullptr;
	}
	
	if (nullptr != m_decodeAudioBuffer)
	{
		delete[] m_decodeAudioBuffer;
		m_decodeAudioBuffer = nullptr;
	}

	if (nullptr != m_audioBuffer)
	{
		delete[] m_audioBuffer;
		m_audioBuffer = nullptr;
	}
}

int XFFmpeg::Input(int avtype, void* data, int len)
{
	if(IsVideo(avtype))
		return InputVideo(avtype, data, len);
	else if(IsAudio(avtype))
		return InputAudio(avtype, data, len);

	return -1;
}

int XFFmpeg::InputVideo(int avtype, void* data, int len)
{
	int ret = 0;
	if (m_pCodecCtxVideo == NULL)
	{
		ret = CreateVideo(avtype);
		if (0 != ret)
		{
			CloseVideo();
			return ret;
		}
	}

	AVPacket packet;
	av_init_packet(&packet);
	packet.data = (uint8_t*)data;
	packet.size = len;

	int got_picture = 0;
	if (avcodec_decode_video2(m_pCodecCtxVideo, m_pFrameVideo, &got_picture, &packet) < 0)
	{
		printf("%s:%d| Decode Error\n", __FUNCTION__, __LINE__);
		return -1;
	}

	if (got_picture == 0)
		return 0;

	// 必须先解码，然后再判断缓存区是否满
	{
		RecursiveGuard mtx(rVideoMutex);
		if (m_count >= YUV_BUFSIZE)
		{
			printf("%s:%d| yuv buffer is full\n", __FUNCTION__, __LINE__);
			return -1;
		}
	}

	if (m_frameBuf[m_head].w != m_pFrameVideo->width || m_frameBuf[m_head].h != m_pFrameVideo->height)
	{
		if (m_frameBuf[m_head].y != NULL)
			free(m_frameBuf[m_head].y);

		if (m_picture != NULL)
		{
			av_frame_free(&m_picture);
			m_picture = NULL;
		}

		if (m_pictureBuf != NULL)
		{
			delete[] m_pictureBuf;
			m_pictureBuf = NULL;
		}

		if (m_imgConvertCtx != NULL)
		{
			sws_freeContext(m_imgConvertCtx);
			m_imgConvertCtx = NULL;
		}
		
		m_ySize = m_pFrameVideo->linesize[0]/*m_pFrameVideo->width*/ * m_pFrameVideo->height;
		int frameSize = avpicture_get_size(AV_PIX_FMT_YUV420P, m_pFrameVideo->linesize[0]/*m_pFrameVideo->width*/, m_pFrameVideo->height);
		m_frameBuf[m_head].y = (unsigned char*)malloc(frameSize);
		if (m_frameBuf[m_head].y == NULL)
		{
			m_frameBuf[m_head].w = 0;
			m_frameBuf[m_head].h = 0;
			return -1;
		}

		m_frameBuf[m_head].w = m_pFrameVideo->width;
		m_frameBuf[m_head].h = m_pFrameVideo->height;
		m_frameBuf[m_head].u = m_frameBuf[m_head].y + m_ySize;
		m_frameBuf[m_head].v = m_frameBuf[m_head].u + m_ySize / 4;
	}

	if (m_pCodecCtxVideo->pix_fmt != AV_PIX_FMT_YUV420P)
	{
		if (m_imgConvertCtx == NULL)
		{
			m_imgConvertCtx = sws_getContext(m_pFrameVideo->width, m_pFrameVideo->height, m_pCodecCtxVideo->pix_fmt,
				m_pFrameVideo->width, m_pFrameVideo->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
			if (m_imgConvertCtx == NULL)
			{
				printf("%s:%d| sws_getContext failed\n", __FUNCTION__, __LINE__);
				return -1;
			}

			int frameSize = avpicture_get_size(AV_PIX_FMT_YUV420P, m_pFrameVideo->width, m_pFrameVideo->height);
			m_picture = av_frame_alloc();
			m_pictureBuf = new uint8_t[frameSize];

			avpicture_fill((AVPicture *)m_picture, m_pictureBuf, AV_PIX_FMT_YUV420P, m_pFrameVideo->width, m_pFrameVideo->height);
		}

		sws_scale(m_imgConvertCtx, (const uint8_t* const*)m_pFrameVideo->data, m_pFrameVideo->linesize, 0,
			m_pFrameVideo->height, m_picture->data, m_picture->linesize);

		memcpy(m_frameBuf[m_head].y, m_picture->data[0], m_ySize);
		memcpy(m_frameBuf[m_head].u, m_picture->data[1], m_ySize / 4);
		memcpy(m_frameBuf[m_head].v, m_picture->data[2], m_ySize / 4);
	}
	else
	{
		memcpy(m_frameBuf[m_head].y, m_pFrameVideo->data[0], m_ySize);
		memcpy(m_frameBuf[m_head].u, m_pFrameVideo->data[1], m_ySize / 4);
		memcpy(m_frameBuf[m_head].v, m_pFrameVideo->data[2], m_ySize / 4);
	}

	m_frameBuf[m_head].stride_y = m_pFrameVideo->linesize[0];
	m_frameBuf[m_head].stride_uv = m_pFrameVideo->linesize[1];
	//m_frameBuf[m_head].pts = pack->pts;

	m_head = (m_head + 1) % YUV_BUFSIZE;
	{
		RecursiveGuard mtx(rVideoMutex);
		++m_count;
	}

	return 0;
}

int XFFmpeg::InputAudio(int avtype, void* data, int datalen)
{
	RecursiveGuard mtx(rAudioMutex);
	int ret = 0, len = 0;
	if (nullptr == m_pCodecCtxAudio)
	{
		ret = CreateAudio(nullptr, avtype);
		if (0 != ret)
		{
			CloseAudio();
			return ret;
		}
	}

	// 解码音频流
	m_audioPacket->data = (uint8_t*)data;
	m_audioPacket->size = datalen;

	ret = avcodec_send_packet(m_pCodecCtxAudio, m_audioPacket);
	if (ret < 0) 
	{
		av_packet_unref(m_audioPacket);
		return -1;
	}
	av_packet_unref(m_audioPacket);

	while (ret >= 0)
	{
		ret = avcodec_receive_frame(m_pCodecCtxAudio, m_frameAudio);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
		{
			return -1;
		}
		else if (ret < 0)
		{
			return -1;
		}

		if (nullptr == m_audioSwrCtx)
		{//初始化转换
			m_nInChannels = AV_CH_LAYOUT_MONO;
			m_audioSwrCtx = swr_alloc();

			if (m_frameAudio->channels == 1)
				m_nInChannels = AV_CH_LAYOUT_MONO; //单声道
			else if (m_frameAudio->channels == 2 || m_frameAudio->channels == 3)
				m_nInChannels = AV_CH_LAYOUT_STEREO;//立体声

			in_channel_layout = av_get_default_channel_layout(
				m_frameAudio->channels);

			swr_alloc_set_opts(m_audioSwrCtx, m_nInChannels, AV_SAMPLE_FMT_S16,
				m_frameAudio->sample_rate,
				in_channel_layout, m_pCodecCtxAudio->sample_fmt, m_frameAudio->sample_rate, 0, NULL);
			swr_init(m_audioSwrCtx);
		}

		len = swr_convert(m_audioSwrCtx, &m_audioBuffer, MaxOneDecodeAudioLength,
			(const uint8_t**)m_frameAudio->data,
			m_frameAudio->nb_samples);

		int out_channel_nb = av_get_channel_layout_nb_channels(m_nInChannels);

		m_oneAudioFrameLength = m_frameAudio->channels * (m_frameAudio->sample_rate / 100) * 2; //每次拷贝10毫秒长度

		m_audioOutBufferSize = av_samples_get_buffer_size(NULL, out_channel_nb,
			m_frameAudio->nb_samples,
			AV_SAMPLE_FMT_S16, 0);

		if (ret == 0)
			break;
	}

	if (m_audioOutBufferSize > 0 && MaxDecodeAudioBufferCache - m_decodeAudioLength >= m_audioOutBufferSize && m_oneAudioFrameLength > 0)
	{
		memcpy(m_decodeAudioBuffer + m_decodeAudioLength, m_audioBuffer, m_audioOutBufferSize);
		m_decodeAudioLength += m_audioOutBufferSize;
	}

	return 0;
}

bool XFFmpeg::GetAudioData(unsigned char* audioPCM, int* audioLength)
{
	RecursiveGuard mtx(rAudioMutex);
	bool bRet = false;
	if (m_pCodecCtxAudio && m_decodeAudioBuffer && m_decodeAudioLength >= m_oneAudioFrameLength && m_oneAudioFrameLength > 0)
	{
		memcpy(audioPCM, m_decodeAudioBuffer, m_oneAudioFrameLength);
		memmove(m_decodeAudioBuffer, m_decodeAudioBuffer + m_oneAudioFrameLength, m_decodeAudioLength - m_oneAudioFrameLength);
		m_decodeAudioLength -= m_oneAudioFrameLength;
		bRet = true;
	}
	else
		bRet = false;

	*audioLength = m_oneAudioFrameLength;
	return bRet;
}

void XFFmpeg::OnVideoWorkThread()
{
	while (m_running)
	{
		if (m_count > 0)
		{
			if (NULL != m_pVideoFunc)
				m_pVideoFunc(&m_frameBuf[m_tail], m_userData);
			m_tail = (m_tail + 1) % YUV_BUFSIZE;
			{
				RecursiveGuard mtx(rVideoMutex);
				--m_count;
			}
		}
		else
		{
			_sleep(10);
		}
	}
}