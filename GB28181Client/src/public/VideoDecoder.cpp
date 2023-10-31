#include "VideoDecoder.h"

static int VideoDecoderThread(void* param)
{
	CVideoDecoder* p = (CVideoDecoder*)param;
	if (p)
	{
		p->OnVideoWorkThread();
	}
	return 0;
}

CVideoDecoder::CVideoDecoder(VideoYuvCallBack pVideoFunc, void* userData)
: m_pVideoFunc(pVideoFunc), m_userData(userData)
, m_pCodecCtxVideo(nullptr), m_pCodecVideo(nullptr), m_pFrameVideo(nullptr)
, m_imgConvertCtx(nullptr), m_picture(nullptr), m_pictureBuf(nullptr)
, m_ySize(0), m_count(0), m_head(0), m_tail(0)
, m_running(true)
{
	memset(&m_frameBuf, 0, sizeof(YuvFrame)*YUV_BUFSIZE);

	m_mapVideoCodecConvert[CODEC_VIDEO_MPEG4] = AV_CODEC_ID_MPEG4;
	m_mapVideoCodecConvert[CODEC_VIDEO_H264] = AV_CODEC_ID_H264;
	m_mapVideoCodecConvert[CODEC_VIDEO_H265] = AV_CODEC_ID_H265;

	m_videoThread = std::thread(VideoDecoderThread, this);
}

CVideoDecoder::~CVideoDecoder()
{
	m_running = false;
	if (m_videoThread.joinable())
		m_videoThread.join();
	CloseVideo_();

	{
		RecursiveGuard mtx(rVideoMutex);
		m_count = 0;
		for (int i = 0; i < YUV_BUFSIZE; ++i)
		{
			if (nullptr != m_frameBuf[i].y)
				free(m_frameBuf[i].y);
		}
	}
}

int CVideoDecoder::CreateVideo_(unsigned char codec)
{
	AVCodecID codecID = AV_CODEC_ID_NONE;
	{
		RecursiveGuard mtx(rVideoMutex);
		auto it = m_mapVideoCodecConvert.find((CodecType)codec);
		if (it == m_mapVideoCodecConvert.end())
		{
			printf("%s:%d| not support codec %d\n", __FUNCTION__, __LINE__, codec);
			return -1;
		}

		codecID = it->second;
	}

	m_pCodecVideo = avcodec_find_decoder(codecID);
	if (nullptr == m_pCodecVideo)
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
	if (nullptr == m_pFrameVideo)
	{
		printf("%s:%d| av_frame_alloc failed\n", __FUNCTION__, __LINE__);
		return -1;
	}
	
	return 0;
}

void CVideoDecoder::CloseVideo_()
{
	if (nullptr != m_pCodecCtxVideo)
	{
		avcodec_close(m_pCodecCtxVideo);
		av_free(m_pCodecCtxVideo);
		m_pCodecCtxVideo = nullptr;
	}

	if (nullptr != m_pFrameVideo)
	{
		av_frame_free(&m_pFrameVideo);
		m_pFrameVideo = nullptr;
	}

	if (nullptr != m_picture)
	{
		av_frame_free(&m_picture);
		m_picture = nullptr;
	}

	if (nullptr != m_pictureBuf)
	{
		delete[] m_pictureBuf;
		m_pictureBuf = nullptr;
	}

	if (nullptr != m_imgConvertCtx)
	{
		sws_freeContext(m_imgConvertCtx);
		m_imgConvertCtx = nullptr;
	}
}

int CVideoDecoder::InputVideoData(int avtype, void* data, int len)
{
	if(IsVideo(avtype))
		return InputVideo_(avtype, data, len);

	return -1;
}

int CVideoDecoder::InputVideo_(int avtype, void* data, int len)
{
	int ret = 0;
	if (nullptr == m_pCodecCtxVideo)
	{
		ret = CreateVideo_(avtype);
		if (0 != ret)
		{
			CloseVideo_();
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
		if (nullptr != m_frameBuf[m_head].y)
			free(m_frameBuf[m_head].y);

		if (nullptr != m_picture)
		{
			av_frame_free(&m_picture);
			m_picture = nullptr;
		}

		if (nullptr != m_pictureBuf)
		{
			delete[] m_pictureBuf;
			m_pictureBuf = nullptr;
		}

		if (nullptr != m_imgConvertCtx)
		{
			sws_freeContext(m_imgConvertCtx);
			m_imgConvertCtx = nullptr;
		}
		
		m_ySize = m_pFrameVideo->linesize[0]/*m_pFrameVideo->width*/ * m_pFrameVideo->height;
		int frameSize = avpicture_get_size(AV_PIX_FMT_YUV420P, m_pFrameVideo->linesize[0]/*m_pFrameVideo->width*/, m_pFrameVideo->height);
		m_frameBuf[m_head].y = (unsigned char*)malloc(frameSize);
		if (nullptr == m_frameBuf[m_head].y)
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
		if (nullptr == m_imgConvertCtx)
		{
			m_imgConvertCtx = sws_getContext(m_pFrameVideo->width, m_pFrameVideo->height, m_pCodecCtxVideo->pix_fmt,
				m_pFrameVideo->width, m_pFrameVideo->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
			if (nullptr == m_imgConvertCtx)
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

void CVideoDecoder::OnVideoWorkThread()
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