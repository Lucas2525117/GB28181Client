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
	return 0;;
}

static int OnDemuxerPacket(void* param, int stream, int codecid, int flags, int64_t pts, int64_t dts, const void* data, size_t bytes)
{
	assert(param);
	CPSParse* parse = (CPSParse*)param;
	return parse->Package(stream, codecid, (void*)data, bytes);
}

static void mpeg_ps_dec_testonstream_10000(void* param, int stream, int codecid, const void* extra, int bytes, int finish)
{
	//printf("stream %d, codecid: %d, finish: %s\n", stream, codecid, finish ? "true" : "false");
}

//rtp 解包
struct ps_demuxer_notify_t notify_10000 = { mpeg_ps_dec_testonstream_10000, };

CPSParse::CPSParse(GBDataCallBack dataCB, void* user)
	: m_func(dataCB)
	, m_user(user)
{
	m_ps = ps_demuxer_create(OnDemuxerPacket, this);
	if (m_ps)
		ps_demuxer_set_notify(m_ps, &notify_10000, this);
}

CPSParse::~CPSParse()
{
	if (m_ps)
		ps_demuxer_destroy(m_ps);
}

int CPSParse::InputData(void* data, int len)
{
	if (nullptr == m_ps || nullptr == data || len <= 0)
		return -1;
	
	ps_demuxer_input(m_ps, (const uint8_t*)data, len);
	return 0;
}

void CPSParse::SetBaseTime(int64_t time)
{

}

int CPSParse::Package(int streamid, int codecid, void* data, size_t bytes)
{
	if (!data || 0 == bytes)
		return -1;

	// 暂不处理音频
	/*if (!IsVideo(avtype))
		return -1;*/

	if (m_func)
		m_func(codecid, data, bytes, m_user);
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

