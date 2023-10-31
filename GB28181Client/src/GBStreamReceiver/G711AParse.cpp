#include "G711AParse.h"
#include <time.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

CG711AParse::CG711AParse(int codec, GBDataCallBack dataCB, void* user)
	: m_codec(codec)
	, m_func(dataCB)
	, m_user(user)
{
}

CG711AParse::~CG711AParse()
{
}

int CG711AParse::InputData(void* data, int len)
{
	return Package(0, data, len);
}

int CG711AParse::Package(int streamid, void* data, size_t bytes)
{
	if (!data || 0 == bytes)
		return -1;

	if (m_func)
		m_func(streamid, data, bytes, m_user);
	return 0;
}

