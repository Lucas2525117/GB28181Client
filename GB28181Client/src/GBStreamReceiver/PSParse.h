#ifndef _PS_PARSE_H_
#define _PS_PARSE_H_

#include "mpeg-ps.h"
#include "GBPublic.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

class CPSParse
{
public:
	CPSParse(GBDataCallBack dataCB, void* user);
	~CPSParse();

	int InputData(void* data, int len);

	void SetBaseTime(int64_t time);

	int Package(int streamid, int codecid, void* data, size_t bytes);

private:
	int64_t time64_now();

private:
	//ps_muxer_t* m_ps = nullptr;
	//int m_ps_stream;
	//int64_t m_ps_clock;

	ps_demuxer_t* m_ps = nullptr;

	GBDataCallBack m_func;
	void* m_user;
};

#endif

