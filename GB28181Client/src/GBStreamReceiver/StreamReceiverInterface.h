#ifndef _STREAMRECEIVER_INTERFACE_H_
#define _STREAMRECEIVER_INTERFACE_H_

#if defined(GB_STREAM__DECL)
	#undef GB_STREAM_DECL
	#define GB_STREAM_DECL
#else
	#ifdef WIN32
		#ifdef GB_STREAM_EXPORTS
			#define GB_STREAM_DECL	__declspec(dllexport)
		#else
			#define GB_STREAM_DECL	__declspec(dllimport)
		#endif
	#else
		#if defined(__GNUC__) && __GNUC__>=4
			#define GB_STREAM_DECL	__attribute__((visibility("default")))
		#else
			#define GB_STREAM_DECL
		#endif
	#endif
#endif

#ifdef __cplusplus
	#define GB_STREAM_API extern "C" GB_STREAM_DECL
#else
	#define GB_STREAM_API GB_STREAM_DECL
#endif

#include "GBPublic.h"

struct IStreamReceiver
{
protected:
	virtual ~IStreamReceiver()  {}

public:
	// 对象删除
	virtual void DeleteThis() = 0;

	// 开始收流 
	// streamType 0:视频流 1:音频流
	virtual int Start(int streamType) = 0;

	// 停止收流
	virtual int Stop() = 0;

	// 设置码流类型
	virtual int SetCodec(int codec) = 0;
};

// 收流url gbudp://192.168.1.2:36000
GB_STREAM_API IStreamReceiver* GB_CreateStreamReceiver(const char* gbUrl, GBDataCallBack func, void* userParam);

#endif
