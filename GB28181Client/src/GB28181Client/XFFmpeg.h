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
}

typedef void(*VideoYuvCallBack)(const YuvFrame* yuv, void* userData);

#define YUV_BUFSIZE (20)

class XFFmpeg
{
public:
	XFFmpeg(VideoYuvCallBack pVideoFunc, void* userData);
	virtual ~XFFmpeg();

public:
	int Input(int avtype, void* data, int len);
	
	void OnWorkThread();

private:
	int CreateVideo(unsigned char codec);

	void CloseVideo();

	int InputVideo(int avtype, void* data, int len);

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

	typedef std::lock_guard<std::recursive_mutex> RecursiveGuard;
	std::recursive_mutex m_recursive_mutex;   //µÝ¹éËø

	std::thread m_thread;
	bool m_running;
};

#endif // _X_FFMPEG_H_
