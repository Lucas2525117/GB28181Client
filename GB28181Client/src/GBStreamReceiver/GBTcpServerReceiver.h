#ifndef _GB_TCP_SERVER_RECEIVER_H_
#define _GB_TCP_SERVER_RECEIVER_H_

#include "StreamReceiverInterface.h"
#include "TcpServer.h"
#include "Rtp2PS.h"
#include "PSParse.h"
#include <string>
#include <thread>

class CGBTcpServerStreamReceiver : public IStreamReceiver
{
public:
	CGBTcpServerStreamReceiver(const char* gbUrl, GBDataCallBack func, void* userParam);
	virtual ~CGBTcpServerStreamReceiver();

	void InputTcpData(void* data, int len);
	void InputPSData(void* data, int len);

public:
	virtual void DeleteThis() override;

	// 开始收流
	virtual int Start(int streamType) override;

	// 停止收流
	virtual int Stop() override;

private:
	int ParseUrl_(const std::string& gburl);

private:
	GBDataCallBack    m_func;
	void*             m_userdata;

	std::string       m_gbUrl;
	std::string       m_localIP;
	int               m_localPort = 0;

	TcpServerPtr      m_tcpServer;
	CRtp2Ps           m_rtp2PS;
	std::shared_ptr<CPSParse> m_parse;
};

#endif  // _GB_TCP_SERVER_RECEIVER_H_

