#include "Rtp2PS.h"
#include <WinSock2.h>

CRtp2Ps::CRtp2Ps(PsDataCallBack pFunc, void* userData)
: m_func(pFunc), m_userData(userData)
, m_payload(0), m_lastPts(-1)
, m_inputBufSize(0), m_inputDataLen(0)
{
}

CRtp2Ps::~CRtp2Ps()
{
}

INT32 CRtp2Ps::InputData(void* data, INT32 len)
{
	if (!m_func || len < 12)
		return -1;

	RTP_HEADER_ *pRtpHeader = (RTP_HEADER_*)data;
	if (0 != pRtpHeader->flagBit.payloadtype)
		m_payload = pRtpHeader->flagBit.payloadtype;
	UINT32 pts = ntohl(pRtpHeader->timeStamp);
	
	char *pdata = (char *)data + 12 + pRtpHeader->flagBit.csrccount*4;
	len = len - 12 - pRtpHeader->flagBit.csrccount*4;

	// rtp ext head
	if (pRtpHeader->flagBit.extension)
	{
		INT32 s = pdata[2]<<8|pdata[3];
		pdata += (4 + s * 4);
		len -= (4 + s * 4);
	}	
	
	if (pRtpHeader->flagBit.padding)
	{
		len -= pdata[len-1];
	}

	if (len <= 0)
		return -1;

	if (m_inputBufSize < m_inputDataLen+len)
	{
		auto ptr = std::make_shared<char>(m_inputBufSize + len + 8192);
		if (!ptr.get())
			return -1;
		if(m_inputBuf.get())
		{
			memcpy(ptr.get(), m_inputBuf.get(), m_inputDataLen);
		}

		m_inputBuf = ptr;
		m_inputBufSize += len + 8192;
	}

	if (pRtpHeader->flagBit.marker)
	{
		memcpy(m_inputBuf.get() + m_inputDataLen, pdata, len);
		m_inputDataLen += len;
		m_func(m_inputBuf.get(), m_inputDataLen, m_userData);

		m_inputDataLen = 0;
	}
	else if (m_lastPts != pts && m_lastPts != -1)
	{
		if (m_inputDataLen > 0)
			m_func(m_inputBuf.get(), m_inputDataLen, m_userData);

		memcpy(m_inputBuf.get(), pdata, len);
		m_inputDataLen = len;
	}
	else
	{
		memcpy(m_inputBuf.get() + m_inputDataLen, pdata, len);
		m_inputDataLen += len;
	}

	m_lastPts = pts;

	return 0;
}
