#include "XFFmpeg.h"
#include <map>

std::map<CodecType, AVCodecID> g_codecConvert;

static int DecoderThread(void* param)
{
	XFFmpeg* p = (XFFmpeg*)param;
	if (p)
	{
		p->OnWorkThread();
	}
	return 0;
}

XFFmpeg::XFFmpeg(VideoYuvCallBack pVideoFunc, void* userData)
: m_pVideoFunc(pVideoFunc), m_userData(userData)
, m_pCodecCtxVideo(NULL), m_pCodecVideo(NULL), m_pFrameVideo(NULL)
, m_imgConvertCtx(NULL), m_picture(NULL), m_pictureBuf(NULL)
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

	m_thread = std::thread(DecoderThread, this);
}

XFFmpeg::~XFFmpeg()
{
	m_running = false;
	if (m_thread.joinable())
		m_thread.join();
	CloseVideo();

	{
		RecursiveGuard mtx(m_recursive_mutex);
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

int XFFmpeg::Input(int avtype, void* data, int len)
{
	return InputVideo(avtype, data, len);
}

int XFFmpeg::InputVideo(int avtype, void* data, int len)
{
	int ret = 0;
	if (m_pCodecCtxVideo == NULL)
	{
		ret = CreateVideo(avtype);
		if (ret != 0)
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
		RecursiveGuard mtx(m_recursive_mutex);
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
		RecursiveGuard mtx(m_recursive_mutex);
		++m_count;
	}

	return 0;
}

void XFFmpeg::OnWorkThread()
{
	while (m_running)
	{
		if (m_count > 0)
		{
			if (NULL != m_pVideoFunc)
				m_pVideoFunc(&m_frameBuf[m_tail], m_userData);
			m_tail = (m_tail + 1) % YUV_BUFSIZE;
			{
				RecursiveGuard mtx(m_recursive_mutex);
				--m_count;
			}
		}
		else
		{
			_sleep(10);
		}
	}
}
