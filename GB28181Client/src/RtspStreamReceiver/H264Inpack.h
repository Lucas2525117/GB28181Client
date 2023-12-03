#ifndef _rtsp_h264_inpack_h_
#define _rtsp_h264_inpack_h_

#include <stdlib.h>
#include <vector>
#include <list>
#include <string.h>
#include <algorithm>
#include "StreamPublic.h"
#include "XXXInpack.h"

enum H264_NAL_TYPE
{
	H264_NAL_SLICE  = 1,
	H264_NAL_DPA = 2,
	H264_NAL_DPB = 3,
	H264_NAL_DPC = 4,
	H264_NAL_IDR = 5,
	H264_NAL_SEI = 6,
	H264_NAL_SPS = 7,
	H264_NAL_PPS = 8
};

class H264Inpack : public XXX2InPack
{
public:
	H264Inpack(StreamDataCallBack func, void* userData, int avtype, bool bVideo);
	virtual ~H264Inpack();

	virtual int InputData(void *data, int len, int64_t pts);

private:
	int Package(void *data, int len, H264_NAL_TYPE nalType, int64_t pts);

private:
	std::shared_ptr<char> m_ptr;
    int m_capacity;

	StreamDataCallBack m_func;
	void*              m_userData;
	int                m_avtype;

	int                m_spslen;
	char               m_sps[128];
	int                m_ppslen;
	char               m_pps[64];
	int                m_seilen;
	char               m_sei[64];

	bool               m_bVideo;
};

#endif /* !_h264_file_reader_h_ */
