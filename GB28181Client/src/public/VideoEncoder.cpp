#include "VideoEncoder.h"

static unsigned int CheckSum(const unsigned int* data, int len)
{
	unsigned int sum = 0;
	for(int i=0; i<len; ++i)
	{
		sum += data[i];
	}

	return sum;
}

static int EncoderThread(void* param)
{
	CStreamEncoder* p = (CStreamEncoder*)param;
	if (p)
	{
		p->OnEncoderWork();
		return 0;
	}
	return -1;
}

CStreamEncoder::CStreamEncoder(EncoderDataCallBack func, void* userData)
	: m_running(true), m_sWidth(0), m_sHeight(0)
	, m_tWidth(0), m_tHeight(0), m_frameSize(0), m_ySize(0), m_codec(0)
	, m_pCodecCtx(nullptr), m_imgConvertCtx(nullptr), m_fifoVideo(nullptr), m_picture(nullptr), m_pictureBuf(nullptr)
	, m_func(func), m_userData(userData)
{
	m_tWidth = (1920 + 0xf) & ~0xf;   // modify???
	m_tHeight = (1080 + 0xf) & ~0xf;  // modify???
	m_ySize = m_tWidth * m_tHeight;
	m_frameSize = avpicture_get_size(AV_PIX_FMT_YUV420P, m_tWidth, m_tHeight);
	//申请帧缓存  
	m_fifoVideo = av_fifo_alloc(50 * m_frameSize);
}

CStreamEncoder::~CStreamEncoder()
{
	m_running = false;
	Stop();

	if (nullptr != m_pCodecCtx)
	{
		avcodec_close(m_pCodecCtx);
		av_free(m_pCodecCtx);
		m_pCodecCtx = nullptr;
	}

	if (nullptr != m_fifoVideo)
	{
		av_fifo_free(m_fifoVideo);
		m_fifoVideo = nullptr;
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

int CStreamEncoder::Init(int sWidth, int sHeight, AVPixelFormat pixFmt)
{
	m_sWidth = sWidth;
	m_sHeight = sHeight;

	if (sWidth != m_tWidth || sHeight!=m_tHeight)
	{
		m_imgConvertCtx = sws_getContext(sWidth, sHeight, pixFmt, m_tWidth, m_tHeight, AV_PIX_FMT_YUV420P, SWS_BICUBIC, nullptr, nullptr, nullptr);
		if (nullptr == m_imgConvertCtx)
		{
			return -1;
		}

		m_picture = av_frame_alloc();
		m_pictureBuf = new uint8_t[m_frameSize];
		avpicture_fill((AVPicture *)m_picture, m_pictureBuf, AV_PIX_FMT_YUV420P, m_tWidth, m_tHeight);
	}

	return OpenOutPut_();
}

int CStreamEncoder::OpenOutPut_()
{
	std::string encode = "264";  //modify???
	AVCodec *codec = NULL;
	if (encode == "H265" || encode == "h265" || encode == "265")
	{
		m_codec = CODEC_VIDEO_H265;
		codec = avcodec_find_encoder(AV_CODEC_ID_H265);
	}
	else if (encode == "MPEG4" || encode == "mpeg4")
	{
		m_codec = CODEC_VIDEO_MPEG4;
		codec = avcodec_find_encoder(AV_CODEC_ID_MPEG4);
	}
	else
	{
		m_codec = CODEC_VIDEO_H264;
		codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	}
	if (!codec)
	{
		return -1;
	}

	m_pCodecCtx = avcodec_alloc_context3(codec);
	if (!m_pCodecCtx)
	{
		return -1;
	}

	m_pCodecCtx->bit_rate = 4*1024*1024;   // 4M 
	m_pCodecCtx->width = m_tWidth;
	m_pCodecCtx->height = m_tHeight;
	m_pCodecCtx->time_base.num = 1;
	m_pCodecCtx->time_base.den = 25;       // 25帧率
	m_pCodecCtx->gop_size = 50;
	m_pCodecCtx->frame_number = 0;
	m_pCodecCtx->max_b_frames = -1;
	m_pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
	m_pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	// ultrafast, superfast, veryfast, faster, fast, medium slow, slower, veryslow, placebo
	av_opt_set(m_pCodecCtx->priv_data, "preset", "faster", 0);
	av_opt_set(m_pCodecCtx->priv_data, "tune", "zerolatency", 0);
	// baseline, extended, main, high
	av_opt_set(m_pCodecCtx->priv_data, "profile", "main", 0);

	m_pCodecCtx->thread_count = 2;

	int ret = avcodec_open2(m_pCodecCtx, codec, NULL);
	if (ret < 0)
	{
		return -1;
	}

	return ret;
}

int CStreamEncoder::Start()
{	
	m_thread = std::thread(EncoderThread, this);

	return -1;
}

int CStreamEncoder::Stop()
{
	m_running = false;
	if (m_thread.joinable())
		m_thread.join();

	return 0;
}

int CStreamEncoder::InputData(uint8_t * srcSlice[], const int srcSize[])
{
	RecursiveGuard mtx(mutex_);
	if (av_fifo_space(m_fifoVideo) < m_frameSize)
	{
		return -1;
	}

	if (m_imgConvertCtx)
	{
		uint8_t *data[AV_NUM_DATA_POINTERS] = { nullptr,};
		int linesize[AV_NUM_DATA_POINTERS] = { 0, };
		data[0] = srcSlice[0];
		data[1] = srcSlice[1];
		data[2] = srcSlice[2];
		linesize[0] = m_sWidth;
		linesize[1] = m_sWidth/2;
		linesize[2] = m_sWidth/2;

		sws_scale(m_imgConvertCtx, (const uint8_t* const*)data, linesize, 0,
			m_sHeight, m_picture->data, m_picture->linesize);

		av_fifo_generic_write(m_fifoVideo, m_picture->data[0], m_ySize, nullptr);
		av_fifo_generic_write(m_fifoVideo, m_picture->data[1], m_ySize / 4, nullptr);
		av_fifo_generic_write(m_fifoVideo, m_picture->data[2], m_ySize / 4, nullptr);
	}
	else
	{
		av_fifo_generic_write(m_fifoVideo, srcSlice[0], srcSize[0], nullptr);
		av_fifo_generic_write(m_fifoVideo, srcSlice[1], srcSize[1], nullptr);
		av_fifo_generic_write(m_fifoVideo, srcSlice[2], srcSize[2], nullptr);
	}

	return 0;
}

void CStreamEncoder::OnEncoderWork()
{
	AVFrame* picture = av_frame_alloc();
	uint8_t* picture_buf = new uint8_t[m_frameSize];
	char* dataBuf = NULL;
	int bufSize = 0;
	unsigned int seqNum = 0;

	picture->width  = m_tWidth;
	picture->height = m_tHeight;
	picture->format = AV_PIX_FMT_YUV420P;
	picture->sample_aspect_ratio.num = 1;
	picture->sample_aspect_ratio.den = 25;  // 25 modify???
	avpicture_fill((AVPicture *)picture, picture_buf, AV_PIX_FMT_YUV420P, m_tWidth, m_tHeight);

	int index = 0;
	int ret = 0;
	int got_picture = 0;
	while (m_running)
	{
		if (av_fifo_size(m_fifoVideo) >= m_frameSize)
		{
			{
				// read data from fifo
				RecursiveGuard mtx(mutex_);
				av_fifo_generic_read(m_fifoVideo, picture_buf, m_frameSize, nullptr);
			}

			picture->pts = index * 40000;

			AVPacket pkt;
			av_init_packet(&pkt);
			pkt.data = NULL;
			pkt.size = 0;

			ret = avcodec_encode_video2(m_pCodecCtx, &pkt, picture, &got_picture);
			if (ret < 0)
			{
				// 编码错误 
				continue;
			}

			if (1 == got_picture)
			{
				if (bufSize < pkt.size+100)
				{
					char* p = (char*)malloc(pkt.size+100);
					if (NULL != p)
					{
						free(dataBuf);
						dataBuf = p;
					}
				}

				memset(dataBuf, 0, sizeof(InPack));
				InPack* pack = (InPack*)dataBuf;
				pack->codec = m_codec;
				pack->seqNum = seqNum++;
				pack->pts = time(nullptr);
				pack->dataLen = pkt.size;
				memcpy(pack->data, pkt.data, pkt.size);
				EncodeStreamInPack(pack, pkt.flags&AV_PKT_FLAG_KEY ? 1 : 0);
				// 计算从totalLen开始的校验和，并将校验和保存到最后四个保留字节中
				unsigned int sum = CheckSum((const unsigned int*)dataBuf, pack->totalLen/4);
				*(unsigned int*)(dataBuf+pack->totalLen) = sum;

				if(m_func)
					m_func(pack, m_userData);

				av_free_packet(&pkt);
			}

			++index;
		}
		else
		{
			_sleep(2);
		}
	}

	av_frame_free(&picture);
	delete[] picture_buf;
}
