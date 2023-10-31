#ifndef _AUDIO_DECODER_H_
#define _AUDIO_DECODER_H_

#include <mutex>
#include <map>
#include <thread>
#include "StreamDefine.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libavutil/audio_fifo.h"
#include "libavutil/frame.h"
#include "libswresample/swresample.h"
}

#define MaxDecodeAudioBufferCache (1024*512)   //�����������Ƶ��������Ȼ���� (OutAudioSampleRate / 100 ) * 2���ֽ��и�
#define MaxOneDecodeAudioLength   (1024*256)   //��Ƶ��������Ļ��峤��
#define AUDIO_STREAM_DATA_SZIE    (512*1024)

typedef void(*AudioDataCallBack)(const void* data, int datalen, void* userData);

class CAudioDecoder
{
public:
	CAudioDecoder(AudioDataCallBack audioDataCB, void* userdata);
	virtual ~CAudioDecoder();

	int InputAudioData(int avtype, void* data, int datalen);

	void OnAudioWorkThread();

private:
	int CreateAudio_(unsigned char codec);

	void CloseAudio_();

	int InputAudio_(int avtype, void* data, int len);

private:
	AudioDataCallBack m_audioDataCB;
	void*             m_userData;

	AVCodecContext*  m_pCodecCtxAudio;
	AVCodec*         m_pCodecAudio;
	AVFrame*         m_frameAudio;
	AVPacket*        m_audioPacket;

	// ��Ƶ���
	SwrContext*      m_audioSwrCtx;
	unsigned char*   m_decodeAudioBuffer;
	unsigned char*   m_audioBuffer;
	unsigned int     m_oneAudioFrameLength; //һ֡��Ƶ����
	unsigned int     m_decodeAudioLength;
	int              m_nInChannels;
	int              m_channel_layout;
	int              m_audioOutBufferSize;

	typedef std::map<CodecType, AVCodecID> AudioCodecConvertMap;
	AudioCodecConvertMap m_mapAudioCodecConvert;
	typedef std::lock_guard<std::recursive_mutex> RecursiveGuard;
	std::recursive_mutex rAudioMutex;
	bool m_running = true;
	std::thread m_thread;
};

#endif // _AUDIO_DECODER_H_
