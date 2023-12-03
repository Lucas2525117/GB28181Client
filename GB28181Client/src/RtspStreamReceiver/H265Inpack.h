#ifndef _rtsp_h265_inpack_h_
#define _rtsp_h265_inpack_h_

#include <stdlib.h>
#include <vector>
#include <list>
#include <string.h>
#include <algorithm>
#include "XXXInpack.h"
#include "StreamPublic.h"

enum H265_NAL_TYPE
{ 
	H265_NAL_SLICE = 1,
	H265_NAL_IDR_W_RADL = 19,   // I帧
	H265_NAL_IDR_N_LP= 20,      // P帧
	H265_NAL_VPS = 32,          // 视频参数集
	H265_NAL_SPS = 33,          // 序列参数集
	H265_NAL_PPS = 34,          // 图像参数集
	H265_NAL_SEI = 39           // 补充增强信息
};

class H265Inpack : public XXX2InPack
{
public:
	H265Inpack(StreamDataCallBack func, void* userData, int avtype, bool bVideo);
	virtual ~H265Inpack();

	virtual int InputData(void* data, int len, int64_t pts);

private:
	int Package(void *data, int len, H265_NAL_TYPE nalType, int64_t pts);

private:
	std::shared_ptr<char> m_ptr;
	int m_capacity;

	StreamDataCallBack m_func;
	void*              m_userData;
	int                m_avtype;

	int                m_vpslen;
	char               m_vps[128];
	int                m_spslen;
	char               m_sps[128];
	int                m_ppslen;
	char               m_pps[64];
	int                m_seilen;
	char               m_sei[64];

	bool               m_bVideo;
};

#endif /* !_rtsp_h265_inpack_h_ */
