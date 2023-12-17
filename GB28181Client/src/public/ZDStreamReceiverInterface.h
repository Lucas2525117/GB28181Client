#ifndef _ZD_STREAMRECEIVER_INTERFACE_H_
#define _ZD_STREAMRECEIVER_INTERFACE_H_

#if defined(ZD_STREAM_DECL)
	#undef ZD_STREAM_DECL
	#define ZD_STREAM_DECL
#else
	#ifdef WIN32
		#ifdef ZD_STREAM_EXPORTS
			#define ZD_STREAM_DECL	__declspec(dllexport)
		#else
			#define ZD_STREAM_DECL	__declspec(dllimport)
		#endif
	#else
		#if defined(__GNUC__) && __GNUC__>=4
			#define ZD_STREAM_DECL	__attribute__((visibility("default")))
		#else
			#define ZD_STREAM_DECL
		#endif
	#endif
#endif

#ifdef __cplusplus
	#define ZD_STREAM_API extern "C" ZD_STREAM_DECL
#else
	#define ZD_STREAM_API ZD_STREAM_DECL
#endif

#include "StreamPublic.h"

struct IZDStreamReceiver
{
protected:
	virtual ~IZDStreamReceiver()  {}

public:
	// ����ɾ��
	virtual void DeleteThis() = 0;

	// ��ʼ���� 
	// streamType 0:��Ƶ�� 1:��Ƶ��
	virtual int Start(int streamType) = 0;

	// ֹͣ����
	virtual int Stop() = 0;
};

// transType 0:tcp 1:udp
ZD_STREAM_API IZDStreamReceiver* ZD_CreateStreamReceiver(int transType, const char* streamUrl, StreamDataCallBack func, void* userParam);

#endif
