#include "Data2PS.h"
#include <time.h>

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

static void* Alloc(void* /*param*/, size_t bytes)
{
	return malloc(bytes);
}

static void Free(void* /*param*/, void* packet)
{
	free(packet);
}

static int Packet(void* param, int avtype, void* pes, size_t bytes)
{
	assert(param);
	CData2PS* ps = (CData2PS*)param;
	return ps->Package(avtype, pes, bytes);
}

CData2PS::CData2PS(PSDataCallBack psDataCB, void* user)
	: m_func(psDataCB)
	, m_user(user)
{
	struct ps_muxer_func_t func;
	func.alloc = Alloc;
	func.free = Free;
	func.write = Packet;
	m_ps = ps_muxer_create(&func, this);
	// TODO codecid´ý²¹³ä
	m_ps_stream = ps_muxer_add_stream(m_ps, PSI_STREAM_H264, nullptr, 0);
}

CData2PS::~CData2PS()
{
	if (m_ps)
	{
		ps_muxer_destroy(m_ps);
		m_ps = nullptr;
	}
}

int CData2PS::InputData(void* data, int len)
{
	if (!m_ps)
		return -1;

	uint64_t clock = time64_now();
	if (0 == m_ps_clock)
		m_ps_clock = clock;

	return ps_muxer_input(m_ps, m_ps_stream, 0, (clock - m_ps_clock) * 90, (clock - m_ps_clock) * 90, data, len);
}

int CData2PS::Package(int avtype, void* data, size_t bytes)
{
	if (!data || bytes < 0)
		return -1;

	if (m_func)
		m_func(m_ps_clock, data, (int)bytes, m_user);

	return 0;
}

uint64_t CData2PS::time64_now()
{
	uint64_t v;

	FILETIME ft;
	GetSystemTimeAsFileTime((FILETIME*)&ft);
	v = (((__int64)ft.dwHighDateTime << 32) | (__int64)ft.dwLowDateTime) / 10000;
	v -= 0xA9730B66800; 
	return v;
}
