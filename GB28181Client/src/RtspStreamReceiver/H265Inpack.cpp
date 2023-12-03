#include "H265Inpack.h"

#define H265_NAL(v)	 ((v >> 1) & 0x3f)

H265Inpack::H265Inpack(StreamDataCallBack func, void* userData, int avtype, bool bVideo)
: m_ptr(NULL), m_capacity(0)
, m_func(func), m_userData(userData), m_avtype(avtype)
, m_vpslen(0), m_spslen(0), m_ppslen(0), m_seilen(0)
, m_bVideo(bVideo)
{
	memset(m_vps, 0, sizeof(m_vps));
	memset(m_sps, 0, sizeof(m_sps));
	memset(m_pps, 0, sizeof(m_pps));
	memset(m_sei, 0, sizeof(m_sei));
	m_vps[3] = 0x01;
	m_sps[3] = 0x01;
	m_pps[3] = 0x01;
	m_sei[3] = 0x01;
}

H265Inpack::~H265Inpack()
{
}

int H265Inpack::InputData(void* data, int len, int64_t pts)
{
	unsigned char nal = ((unsigned char*)data)[0];
	H265_NAL_TYPE nalType = (H265_NAL_TYPE)H265_NAL(nal);
	switch(nalType)
	{
	case H265_NAL_SLICE:
	case H265_NAL_IDR_N_LP:
	case H265_NAL_IDR_W_RADL:
		Package(data, len, nalType, pts);
		break;
	case H265_NAL_VPS:
		m_vpslen = (size_t)len>(sizeof(m_vps)-4) ? (sizeof(m_vps)-4):len; 
		memcpy(m_vps+4, data, m_vpslen);
		m_vpslen += 4;
		break;
	case H265_NAL_SPS:
		m_spslen = (size_t)len>(sizeof(m_sps)-4) ? (sizeof(m_sps)-4):len;
		memcpy(m_sps+4, data, m_spslen);
		m_spslen += 4;
		break;
	case H265_NAL_PPS:
		m_ppslen = (size_t)len>(sizeof(m_pps)-4) ? (sizeof(m_pps)-4):len;
		memcpy(m_pps+4, data, m_ppslen);
		m_ppslen += 4;
		break;
	case H265_NAL_SEI:
		m_seilen = (size_t)len>(sizeof(m_sei)-4) ? (sizeof(m_sei)-4):len; 
		memcpy(m_sei+4, data, m_seilen);
		m_seilen += 4;
		break;
	default:
		break;
	}

	return 0;
}

// NALU: ÆôÊ¼Âë+VPS+SPS+PPS+SEI+IÖ¡+PÖ¡
int H265Inpack::Package(void* data, int len, H265_NAL_TYPE nalType, int64_t pts)
{
	// ±ÜÃâ¿½±´Ê±Ô½½ç
	if((m_vpslen + m_spslen + m_ppslen + m_seilen) >= 200)
		return -1;
	
	int dataLen = 0;
	int bufSize = len + 4 + 200; 
	if (H265_NAL_IDR_W_RADL == nalType)
	{
		bufSize += m_vpslen + m_spslen + m_ppslen + m_seilen;
	}

	if(m_capacity < bufSize)
	{
		std::shared_ptr<char> buf(new char[bufSize], std::default_delete<char[]>());
		if (!buf.get())
			return -1;

		m_ptr = buf;
	}

	if (H265_NAL_IDR_W_RADL == nalType)
	{
		if(m_vpslen > 0)
		{
			memcpy(m_ptr.get()+dataLen, m_vps, m_vpslen);
			dataLen += m_vpslen;
		}
		if(m_spslen > 0)
		{
			memcpy(m_ptr.get()+dataLen, m_sps, m_spslen);
			dataLen += m_spslen;
		}
		if(m_ppslen > 0)
		{
			memcpy(m_ptr.get()+dataLen, m_pps, m_ppslen);
			dataLen += m_ppslen;
		}
		if(m_seilen > 0)
		{
			memcpy(m_ptr.get()+dataLen, m_sei, m_seilen);
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