#ifndef _VIDEO_ENCODER_H_
#define _VIDEO_ENCODER_H_

#include <thread>
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
}

class CStreamEncoder
{
public:
	CStreamEncoder(EncoderDataCallBack func, void* userData);
	~CStreamEncoder();

	int Init(int width, int height, AVPixelFormat pixFmt);

	int Start();

	int Stop();

	int InputData(uint8_t * srcSlice[], const int srcStride[]);

	void OnEncoderWork();

private:
	int OpenOutPut_();

private:
	bool m_running;
	int m_sWidth;
	int m_sHeight;

	int m_tWidth;
	int m_tHeight;
	int m_frameSize;
	int m_ySize;

	unsigned char    m_codec;
	AVCodecContext*  m_pCodecCtx;
	SwsContext*      m_imgConvertCtx;
	AVFifoBuffer*    m_fifoVideo;
	AVFrame*         m_picture;
	uint8_t*         m_pictureBuf;

	typedef std::lock_guard<std::recursive_mutex> RecursiveGuard;
	std::recursive_mutex mutex_;
	std::thread m_thread;

	EncoderDataCallBack m_func;
	void* m_userData;
};

#endif // _STREAM_ENCODERS_H_
