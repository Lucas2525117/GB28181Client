#ifndef _GB_RTP2PS_H_
#define _GB_RTP2PS_H_

#include "StreamDefine.h"
#include <memory>

class CRtp2Ps
{
public:
	CRtp2Ps(PsDataCallBack func, void* userData);

	~CRtp2Ps();

public:
	int InputData(void* data, int len);

	unsigned char PayloadType() { return m_payload; };

private:
	struct RTP_HEADER_BIT_
	{
		unsigned short csrccount : 4;
		unsigned short extension : 1;
		unsigned short padding : 1;
		unsigned short version : 2;

		unsigned short payloadtype : 7;
		unsigned short marker : 1;
	};

	typedef struct _tagRTP_HEADER
	{
		union
		{
			unsigned short flag;
			RTP_HEADER_BIT_ flagBit;
		};
		unsigned short seq;
		unsigned int timeStamp;
		unsigned int ssrc;
	} RTP_HEADER_;

private:
	PsDataCallBack m_func;
	void* m_userData;

	unsigned char m_payload;
	int64_t m_lastPts;

	std::shared_ptr<char> m_inputBuf;
	int  m_inputBufSize;
	int  m_inputDataLen;
};

#endif
