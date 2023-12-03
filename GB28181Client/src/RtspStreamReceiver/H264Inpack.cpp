#include "H264Inpack.h"

#define H264_NAL(v)	(v & 0x1F)

H264Inpack::H264Inpack(StreamDataCallBack func, void* userData, int avtype, bool bVideo)
: m_ptr(NULL), m_capacity(0)
, m_func(func), m_userData(userData), m_avtype(avtype)
, m_spslen(0), m_ppslen(0), m_seilen(0)
, m_bVideo(bVideo)
{
	memset(m_sps, 0, sizeof(m_sps));
	memset(m_pps, 0, sizeof(m_pps));
	memset(m_sei, 0, sizeof(m_sei));
	m_sps[3] = 0x01;
	m_pps[3] = 0x01;
	m_sei[3] = 0x01;
}

H264Inpack::~H264Inpack()
{
}

int H264Inpack::InputData(void* data, int len, int64_t pts)
{
	// 4 = 00 00 00 01
	unsigned char nal = ((unsigned char*)data)[0];
	H264_NAL_TYPE nalType = (H264_NAL_TYPE)H264_NAL(nal);
	switch(nalType)
	{
	case H264_NAL_SLICE:
	case H264_NAL_DPA:
	case H264_NAL_DPB:
	case H264_NAL_DPC:
	case H264_NAL_IDR:
		Package(data, len, nalType, pts);
		break;
	case H264_NAL_SEI:
		m_seilen = (size_t)len>(sizeof(m_sei)-4) ? (sizeof(m_sei)-4):len; 
		memcpy(m_sei+4, data, m_seilen);
		m_seilen += 4;
		break;
	case H264_NAL_SPS:
		m_spslen = (size_t)len>(sizeof(m_sps)-4) ? (sizeof(m_sps)-4):len;
		memcpy(m_sps+4, data, m_spslen);
		m_spslen += 4;
		break;
	case H264_NAL_PPS:
		m_ppslen = (size_t)len>(sizeof(m_pps)-4) ? (sizeof(m_pps)-4):len;
		memcpy(m_pps+4, data, m_ppslen);
		m_ppslen += 4;
		break;
	default:
		break;
	}

	return 0;
}

int H264Inpack::Package(void* data, int len, H264_NAL_TYPE nalType, int64_t pts)
{
	// ±ÜÃâ¿½±´Ê±Ô½½ç
	if((m_spslen + m_ppslen + m_seilen) >= 100)
		return -1;

	int dataLen = 0;
	int bufSize = len + 4 + 100;
	if (H264_NAL_IDR == nalType)
	{
		bufSize += m_spslen + m_ppslen + m_seilen;
	}

	if(m_capacity < bufSize)
	{
		std::shared_ptr<char> buf(new char[bufSize], std::default_delete<char[]>());
		if(!buf.get())
			return -1;

		memset(buf.get(), 0x00, bufSize);
		m_ptr = buf;
	}

	if (H264_NAL_IDR == nalType)
	{
		if(m_spslen > 0)
		{
			memcpy(m_ptr.get()+dataLen, m_sps, m_spslen);
			dataLen += m_spslen;
		}
		if(m_ppslen > 0)
		{
			memcpy(m_ptr.get() +dataLen, m_pps, m_ppslen);
			dataLen += m_ppslen;
		}
		if(m_seilen > 0)
		{
			memcpy(m_ptr.get() +dataLen, m_sei, m_seilen);
			dataLen += m_seilen;
		}
	}

	m_ptr.get()[dataLen++] = 0x00;
	m_ptr.get()[dataLen++] = 0x00;
	m_ptr.get()[dataLen++] = 0x00;
	m_ptr.get()[dataLen++] = 0x01;

	memcpy(m_ptr.get()+dataLen, data, len);
	dataLen += len;

	if (m_func)
		m_func(m_avtype, m_ptr.get(), dataLen, m_userData);

    return 0;
}
