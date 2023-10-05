#include "PSParse.h"
#include <time.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

static void* Alloc(void* /*param*/, size_t bytes)
{
	return malloc(bytes);
}

static void Free(void* /*param*/, void* packet)
{
	free(packet);
}

static int Write(void* param, int avtype, void* pes, size_t bytes)
{
	assert(param);
	CPSParse* parse = (CPSParse*)param;
	return parse->Package(avtype, pes, bytes);
}

static BOOL IsAudio(int avtype)
{
	switch (avtype)
	{
	case STREAM_AUDIO_AAC:
	case STREAM_AUDIO_G711A:
	case STREAM_AUDIO_G711U:
	case STREAM_AUDIO_G722:
	case STREAM_AUDIO_G723:
	case STREAM_AUDIO_G729:
	case STREAM_AUDIO_SVAC:
		return TRUE;
	default:
		return FALSE;
	}

	return FALSE;
}

static BOOL IsVideo(int avtype)
{
	switch (avtype)
	{
	case STREAM_VIDEO_MPEG4:
	case STREAM_VIDEO_H264:
	case STREAM_VIDEO_H265:
	case STREAM_VIDEO_SVAC:
		return TRUE;
	default:
		return FALSE;
	}

	return FALSE;
}

CPSParse::CPSParse(GBDataCallBack dataCB, void* user)
	: m_func(dataCB)
	, m_user(user)
{
	struct ps_muxer_func_t func;
	func.alloc = Alloc;
	func.free = Free;
	func.write = Write;
	m_ps = ps_muxer_create(&func, this);
	m_ps_stream = ps_muxer_add_stream(m_ps, STREAM_VIDEO_H264, nullptr, 0);
}

CPSParse::~CPSParse()
{
	if (m_ps)
		ps_muxer_destroy(m_ps);
}

int CPSParse::InputData(void* data, int len)
{
	if (nullptr == m_ps || nullptr == data || len <= 0)
		return -1;

	uint64_t clock = time64_now();
	if (0 == m_ps_clock)
		m_ps_clock = clock;
	ps_muxer_input(m_ps, m_ps_stream, 0, (clock - m_ps_clock) * 90, (clock - m_ps_clock) * 90, data, len);
	return 0;
}

void CPSParse::SetBaseTime(int64_t time)
{

}

int CPSParse::Package(int avtype, void* payload, size_t bytes)
{
	if (!payload || 0 == bytes)
		return -1;

	// ÔÝ²»´¦ÀíÒôÆµ
	/*if (!IsVideo(avtype))
		return -1;*/

	if (m_func)
		m_func(avtype, payload, bytes, m_user);
	return 0;
}

int64_t CPSParse::time64_now()
{
	int64_t v;

	FILETIME ft;
	GetSystemTimeAsFileTime((FILETIME*)&ft);
	v = (((__int64)ft.dwHighDateTime << 32) | (__int64)ft.dwLowDateTime) / 10000; // to ms
	v -= 0xA9730B66800; /* 11644473600000LL */ // January 1, 1601 (UTC) -> January 1, 1970 (UTC).

	return v;
}

