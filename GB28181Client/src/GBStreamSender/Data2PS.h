#ifndef _DATA2PS_H_
#define _DATA2PS_H_

#include "mpeg-ps.h"
#include "GBSenderPublic.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

class CData2PS
{
public:
	CData2PS(PSDataCallBack psDataCB, void* user);
	~CData2PS();

public:
	int InputData(void* data, int len);

	int Package(int avtype, void* data, size_t bytes);

private:
	uint64_t time64_now();

private:
	PSDataCallBack m_func;
	void* m_user;

	ps_muxer_t* m_ps = nullptr;
	int m_ps_stream = 0;
	uint64_t m_ps_clock = 0;
};

#endif

