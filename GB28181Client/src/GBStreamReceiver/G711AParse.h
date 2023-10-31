#ifndef _G711A_PARSE_H_
#define _G711A_PARSE_H_

#include "mpeg-ps.h"
#include "GBPublic.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

class CG711AParse
{
public:
	CG711AParse(int codec, GBDataCallBack dataCB, void* user);
	~CG711AParse();

	int InputData(void* data, int len);

	int Package(int streamid, void* data, size_t bytes);

private:
	int m_codec;
	GBDataCallBack m_func;
	void* m_user;
};

#endif // _G711A_PARSE_H_

