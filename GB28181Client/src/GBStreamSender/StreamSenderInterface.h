#ifndef _STREAM_SENDER_INTERFACE_H_
#define _STREAM_SENDER_INTERFACE_H_

#if defined(GB_SENDER_STREAM_DECL)
	#undef GB_SENDER_STREAM_DECL
	#define GB_SENDER_STREAM_DECL
#else
	#ifdef WIN32
		#ifdef GB_SENDER_STREAM_EXPORTS
			#define GB_SENDER_STREAM_DECL	__declspec(dllexport)
		#else
			#define GB_SENDER_STREAM_DECL	__declspec(dllimport)
		#endif
	#else
		#if defined(__GNUC__) && __GNUC__>=4
			#define GB_SENDER_STREAM_DECL	__attribute__((visibility("default")))
		#else
			#define GB_SENDER_STREAM_DECL
		#endif
	#endif
#endif

#ifdef __cplusplus
	#define GB_SENDER_STREAM_API extern "C" GB_SENDER_STREAM_DECL
#else
	#define GB_SENDER_STREAM_API GB_SENDER_STREAM_DECL
#endif

struct IStreamSender
{
protected:
	virtual ~IStreamSender()  {}

public:
	// ����ɾ��
	virtual void DeleteThis() = 0;

	// ��ʼ���� 
	virtual int Start() = 0;

	// ֹͣ����
	virtual int Stop() = 0;

	// ��������
	virtual int SendData(void* data, int len) = 0;

	// ��ȡ��ǰ״̬
	virtual int GetStatus() = 0;
};

// ����url gbudp://192.168.1.2:36000
GB_SENDER_STREAM_API IStreamSender* GB_CreateStreamSender(const char* gbUrl);

#endif // _STREAM_SENDER_INTERFACE_H_
