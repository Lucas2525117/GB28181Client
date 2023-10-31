#ifndef _VIDEO_DECODER_H_
#define _VIDEO_DECODER_H_

#include <mutex>
#include <map>
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

typedef void(*VideoYuvCallBack)(const YuvFrame* yuv, void* userData);

#define YUV_BUFSIZE (20)

class CVideoDecoder
{
public:
	CVideoDecoder(VideoYuvCallBack pVideoFunc, void* userData);
	virtual ~CVideoDecoder();

public:
	int InputVideoData(int avtype, void* data, int datalen);

	void OnVideoWorkThread();

private:
	int CreateVideo_(unsigned char codec);

	void CloseVideo_();

	int InputVideo_(int avtype, void* data, int len);

private:
	VideoYuvCallBack m_pVideoFunc;
	void* m_userData;
	
	AVCodecContext*  m_pCodecCtxVideo;
	AVCodec*         m_pCodecVideo;
	AVFrame*         m_pFrameVideo;

	SwsContext*      m_imgConvertCtx;
	AVFrame*         m_picture;
	uint8_t*         m_pictureBuf;
	int m_ySize;

	YuvFrame m_frameBuf[YUV_BUFSIZE];
	int m_count, m_head, m_tail;

	typedef std::map<CodecType, AVCodecID> VideoCodecConvertMap;
	VideoCodecConvertMap m_mapVideoCodecConvert;
	typedef std::lock_guard<std::recursive_mutex> RecursiveGuard;
	std::recursive_mutex rVideoMutex;   //µÝ¹éËø

	std::thread m_videoThread;
	bool m_running;
};

#endif // _VIDEO_DECODER_H_
