#ifndef _RTSP_STREAMRECEIVER_INTERFACE_H_
#define _RTSP_STREAMRECEIVER_INTERFACE_H_

#if defined(GB_STREAM__DECL)
	#undef GB_STREAM_DECL
	#define GB_STREAM_DECL
#else
	#ifdef WIN32
		#ifdef RTSP_STREAM_EXPORTS
			#define RTSP_STREAM_DECL	__declspec(dllexport)
		#else
			#define RTSP_STREAM_DECL	__declspec(dllimport)
		#endif
	#else
		#if defined(__GNUC__) && __GNUC__>=4
			#define RTSP_STREAM_DECL	__attribute__((visibility("default")))
		#else
			#define RTSP_STREAM_DECL
		#endif
	#endif
#endif

#ifdef __cplusplus
	#define RTSP_STREAM_API extern "C" RTSP_STREAM_DECL
#else
	#define RTSP_STREAM_API RTSP_STREAM_DECL
#endif

#include "StreamPublic.h"

struct IRtspStreamReceiver
{
protected:
	virtual ~IRtspStreamReceiver()  {}

public:
	// 对象删除
	virtual void DeleteThis() = 0;

	// 开始收流 
	// streamType 0:视频流 1:音频流
	virtual int Start(int streamType) = 0;

	// 停止收流
	virtual int Stop() = 0;
};

// 收流url gbudp://192.168.1.2:36000
RTSP_STREAM_API IRtspStreamReceiver* Rtsp_CreateStreamReceiver(const char* gbUrl, StreamDataCallBack func, void* userParam);

#endif
