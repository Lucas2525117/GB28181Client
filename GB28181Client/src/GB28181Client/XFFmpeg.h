#ifndef _X_FFMPEG_H_
#define _X_FFMPEG_H_

#include <mutex>
#include "public.h"

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

typedef void(*VideoYuvCallBack)(const YuvFrame* yuv, void* userData);
typedef void(*AudioDataCallBack)(const char* data, void* userData);

#define YUV_BUFSIZE (20)

// 音频相关
#define MaxDecodeAudioBufferCache (1024*512)   //解码出来的音频缓存区，然后按照 (OutAudioSampleRate / 100 ) * 2个字节切割
#define MaxOneDecodeAudioLength   (1024*256)   //音频解码后最大的缓冲长度

class XFFmpeg
{
public:
	XFFmpeg(VideoYuvCallBack pVideoFunc, AudioDataCallBack pAudioFunc,void* userData);
	virtual ~XFFmpeg();

public:
	int Input(int avtype, void* data, int datalen);

	bool GetAudioData(unsigned char* audioPCM, int* audioLength);
	
	void OnVideoWorkThread();

private:
	int CreateVideo(unsigned char codec);

	void CloseVideo();

	int CreateAudio(const char* fileName, unsigned char codec);

	void CloseAudio();

	int InputVideo(int avtype, void* data, int len);

	int InputAudio(int avtype, void* data, int len);

private:
	VideoYuvCallBack m_pVideoFunc;
	AudioDataCallBack m_pAudioFunc;
	void* m_userData;
	
	AVCodecContext*  m_pCodecCtxVideo;
	AVCodec*         m_pCodecVideo;
	AVFrame*         m_pFrameVideo;

	AVCodecContext*  m_pCodecCtxAudio;
	AVCodec*         m_pCodecAudio;
	AVFrame*         m_frameAudio;
	AVPacket*        m_audioPacket;

	SwsContext*      m_imgConvertCtx;
	AVFrame*         m_picture;
	uint8_t*         m_pictureBuf;
	int m_ySize;

	// 音频相关
	SwrContext*      m_audioSwrCtx;
	unsigned char*   m_decodeAudioBuffer;
	unsigned char*   m_audioBuffer;
	unsigned int     m_oneAudioFrameLength; //一帧音频长度
	unsigned int     m_decodeAudioLength;
	int              m_nInChannels;
	int              in_channel_layout;
	int              m_audioOutBufferSize;

	YuvFrame m_frameBuf[YUV_BUFSIZE];
	int m_count, m_head, m_tail;

	typedef std::lock_guard<std::recursive_mutex> RecursiveGuard;
	std::recursive_mutex rVideoMutex;   //递归锁
	std::recursive_mutex rAudioMutex;

	std::thread m_videoThread;
	std::thread m_audioThread;
	bool m_running;
};

#endif // _X_FFMPEG_H_
