#ifndef _XXXINPACK_H_
#define _XXXINPACK_H_

#include <memory>

class XXX2InPack
{
public:
	virtual ~XXX2InPack() {};
	virtual int InputData(void* data, int len, int64_t pts) = 0;
};

#endif
