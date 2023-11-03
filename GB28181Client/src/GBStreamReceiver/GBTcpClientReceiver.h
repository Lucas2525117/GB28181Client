#ifndef _GB_TCP_CLIENT_RECEIVER_H_
#define _GB_TCP_CLIENT_RECEIVER_H_

#include "StreamReceiverInterface.h"
#include "TcpClient.h"
#include "Rtp2PS.h"
#include "PSParse.h"
#include <string>
#include <thread>

class CGBTcpClientStreamReceiver : public IStreamReceiver
{
public:
	CGBTcpClientStreamReceiver(const char* gbUrl, GBDataCallBack func, void* userParam);
	virtual ~CGBTcpClientStreamReceiver();

	void InputTcpData(void* data, int len);
	void InputPSData(void* data, int len);

public:
	virtual void DeleteThis() override;

	// ��ʼ����
	virtual int Start(int streamType) override;

	// ֹͣ����
	virtual int Stop() override;

	// ������������
	virtual int SetCodec(int codec) override;

private:
	int ParseUrl_(const std::string& gburl);

private:
	GBDataCallBack    m_func;
	void*             m_userdata;

	std::string       m_gbUrl;
	std::string       m_localIP;
	int               m_localPort = 0;

	TcpClientPtr      m_tcpClient;
	CRtp2Ps           m_rtp2PS;
	std::shared_ptr<CPSParse> m_parse;
};

#endif  // _GB_TCP_CLIENT_RECEIVER_H_

