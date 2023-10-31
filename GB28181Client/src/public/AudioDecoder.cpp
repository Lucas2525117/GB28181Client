#include "AudioDecoder.h"

static int AudioDecoderThread(void* param)
{
	CAudioDecoder* p = (CAudioDecoder*)param;
	if (p)
	{
		p->OnAudioWorkThread();
	}
	return 0;
}


CAudioDecoder::CAudioDecoder(AudioDataCallBack audioDataCB, void* userdata)
	: m_audioDataCB(audioDataCB), m_userData(userdata)
, m_pCodecCtxAudio(nullptr), m_pCodecAudio(nullptr), m_frameAudio(nullptr)
, m_audioPacket(nullptr), m_audioSwrCtx(nullptr), m_decodeAudioBuffer(nullptr)
, m_audioBuffer(nullptr)
, m_oneAudioFrameLength(0), m_decodeAudioLength(0), m_nInChannels(0)
, m_channel_layout(0), m_audioOutBufferSize(0)
{
	m_mapAudioCodecConvert[CODEC_AUDIO_G711] = AV_CODEC_ID_PCM_ALAW;
	m_mapAudioCodecConvert[CODEC_AUDIO_G711U] = AV_CODEC_ID_PCM_MULAW;
	m_mapAudioCodecConvert[CODEC_AUDIO_PCM] = AV_CODEC_ID_FIRST_AUDIO;
	m_mapAudioCodecConvert[CODEC_AUDIO_AAC] = AV_CODEC_ID_AAC;

	m_thread = std::thread(AudioDecoderThread, this);
}

CAudioDecoder::~CAudioDecoder()
{
	m_running = false;
	if (m_thread.joinable())
		m_thread.join();

	CloseAudio_();
}

int CAudioDecoder::CreateAudio_(unsigned char codec)
{
	AVCodecID codecID = AV_CODEC_ID_NONE;
	{
		RecursiveGuard mtx(rAudioMutex);
		auto it = m_mapAudioCodecConvert.find((CodecType)codec);
		if (it == m_mapAudioCodecConvert.end())
		{
			printf("%s:%d| not support codec %d\n", __FUNCTION__, __LINE__, codec);
			return -1;
		}

		codecID = it->second;
	}

	// 获取音频解码器
	m_pCodecAudio = avcodec_find_decoder(codecID);
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

	m_pCodecCtxAudio->codec_id = codecID;
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
	if (nullptr == m_decodeAudioBuffer)
	{
		printf("%s:%d| create decode audio buffer failed\n", __FUNCTION__, __LINE__);
		return -1;
	}

	m_audioBuffer = new unsigned char[MaxOneDecodeAudioLength];
	if (nullptr == m_decodeAudioBuffer)
	{
		printf("%s:%d| create audio buffer failed\n", __FUNCTION__, __LINE__);
		return -1;
	}

	return 0;
}

void CAudioDecoder::CloseAudio_()
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

int CAudioDecoder::InputAudioData(int avtype, void* data, int len)
{
	if (IsAudio(avtype))
		return InputAudio_(avtype, data, len);

	return -1;
}

int CAudioDecoder::InputAudio_(int avtype, void* data, int datalen)
{
	RecursiveGuard mtx(rAudioMutex);
	int ret = 0, len = 0;
	if (nullptr == m_pCodecCtxAudio)
	{
		ret = CreateAudio_(avtype);
		if (0 != ret)
		{
			CloseAudio_();
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
		{
			//初始化转换
			m_nInChannels = AV_CH_LAYOUT_MONO;
			m_audioSwrCtx = swr_alloc();

			if (1 == m_frameAudio->channels == 1)
				m_nInChannels = AV_CH_LAYOUT_MONO; //单声道
			else if (2 == m_frameAudio->channels || 3 == m_frameAudio->channels)
				m_nInChannels = AV_CH_LAYOUT_STEREO;//立体声

			m_channel_layout = av_get_default_channel_layout(m_frameAudio->channels);

			swr_alloc_set_opts(m_audioSwrCtx, m_nInChannels, AV_SAMPLE_FMT_S16,
				m_frameAudio->sample_rate,
				m_channel_layout, m_pCodecCtxAudio->sample_fmt, m_frameAudio->sample_rate, 0, NULL);
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

		if (0 == ret)
			break;
	}

	if (m_audioOutBufferSize > 0 && MaxDecodeAudioBufferCache - m_decodeAudioLength >= m_audioOutBufferSize && m_oneAudioFrameLength > 0)
	{
		memcpy(m_decodeAudioBuffer + m_decodeAudioLength, m_audioBuffer, m_audioOutBufferSize);
		m_decodeAudioLength += m_audioOutBufferSize;
	}

	return 0;
}

void CAudioDecoder::OnAudioWorkThread()
{
	while (m_running)
	{
		std::shared_ptr<char> audioData(new char[AUDIO_STREAM_DATA_SZIE], std::default_delete<char[]>());
		memset(audioData.get(), 0x00, AUDIO_STREAM_DATA_SZIE);
		int datalen = 0;

		if (m_pCodecCtxAudio && m_decodeAudioBuffer && m_decodeAudioLength >= m_oneAudioFrameLength && m_oneAudioFrameLength > 0)
		{
			memcpy(audioData.get(), m_decodeAudioBuffer, m_oneAudioFrameLength);
			memmove(m_decodeAudioBuffer, m_decodeAudioBuffer + m_oneAudioFrameLength, m_decodeAudioLength - m_oneAudioFrameLength);
			m_decodeAudioLength -= m_oneAudioFrameLength;
			datalen = m_oneAudioFrameLength;

			if (m_audioDataCB && m_userData)
				m_audioDataCB(audioData.get(), datalen, m_userData);
		}
		else
		{
			_sleep(10);
		}
	}
}
