#ifndef _GB28181_SERVER_H_
#define _GB28181_SERVER_H_

#include <string>
#include "MySipHeader.h"
#include "MySipMedia.h"
#include "SipSDKPublic.h"

#if defined(GB28181_DECL)
	#undef GB28181_DECL
	#define GB28181_DECL
#else
	#ifdef WIN32
		#ifdef GB28181_EXPORTS
			#define GB28181_DECL	__declspec(dllexport)
		#else
			#define GB28181_DECL	__declspec(dllimport)
		#endif
	#else
		#if defined(__GNUC__) && __GNUC__>=4
			#define GB28181_DECL	__attribute__((visibility("default")))
		#else
			#define GB28181_DECL
		#endif
	#endif
#endif

#ifdef __cplusplus
#define GB28181_API extern "C" GB28181_DECL
#else
#define GB28181_API GB28181_DECL
#endif

GB28181_API bool GB_Init(const char* concat, int loglevel);

GB28181_API bool GB_UnInit();

GB28181_API void GB_RegisterHandler(int handleType, DataCallback dataCB, void* user = nullptr);

GB28181_API int GB_PTZControl(const char* gbid, PTZControlType controlType, int paramValue);

GB28181_API const char* GB_Invite(GB28181MediaContext mediaContext);

GB28181_API int GB_QueryNetDeviceInfo(int type, const char* gbid);

GB28181_API void GB_QueryRecordInfo(const char* gbid, const GB28181MediaContext& mediaContext);

GB28181_API bool GB_Bye(const char* token);

#endif

