#ifndef _GB_TCPCLIENT_SENDER_H_
#define _GB_TCPCLIENT_SENDER_H_

#include "StreamSenderInterface.h"
#include "rtpsession.h"
#include "rtpsessionparams.h"
#include "rtptcptransmitter.h"
#include "rtptcpaddress.h"
#include "rtppacket.h"
#include "rtpsources.h"
#include "rtpsourcedata.h"
#include "rtpaddress.h"
#include "rtpipv4address.h"
#include "ZDTcpClient.h"
#include "Data2PS.h"
#include <thread>

using namespace jrtplib;

class CGBTcpClientSender : public IStreamSender, public RTPSession
{
public:
	CGBTcpClientSender(const char* gbUrl);
	virtual ~CGBTcpClientSender();

	void InputPSTCPData(int64_t pts, void* _data, int _len);

	void RTPPackWorker();

public:
	virtual void DeleteThis() override;

	// 开始发流 
	virtual int Start() override;

	// 停止发流
	virtual int Stop() override;

	// 发送数据
	virtual int SendData(void* data, int len) override;

	// 获取状态
	virtual int GetStatus() override;

private:
	int ParseUrl_(const std::string& gburl);
	int InitRtpSession_();

private:
	std::string        m_gbUrl = "";
	std::string        m_localIP = "";
	int                m_localPort = 0;

	ZDTcpClientPtr     m_tcpClient;
	CData2PS*          m_pspacker = nullptr;

	std::thread        m_thread;
	bool               m_running = true;
	RTPTCPTransmitter* m_rtpTcpTransmitter = nullptr;
};

#endif
