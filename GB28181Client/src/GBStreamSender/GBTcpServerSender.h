#ifndef _GB_TCPSERVER_SENDER_H_
#define _GB_TCPSERVER_SENDER_H_

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
#include "ZDTcpServer.h"
#include "Data2PS.h"
#include <thread>
#include <chrono>

using namespace jrtplib;

class CGBTcpServerSender : public IStreamSender, public RTPSession
{
public:
	CGBTcpServerSender(const char* gbUrl);
	virtual ~CGBTcpServerSender();

	void InputPSTCPData(int64_t pts, void* _data, int _len);

	void TCPData2PSWorker();

public:
	virtual void DeleteThis() override;

	// ��ʼ���� 
	virtual int Start() override;

	// ֹͣ����
	virtual int Stop() override;

	// ��������
	virtual int SendData(void* data, int len) override;

	// ��ȡ״̬
	virtual int GetStatus() override;

private:
	int ParseUrl_(const std::string& gburl);
	int InitRtpSession_();

private:
	std::string        m_gbUrl = "";
	std::string        m_localIP = "";
	int                m_localPort = 0;

	ZDTcpServerPtr     m_tcpServer;
	CData2PS*          m_pspacker = nullptr;

	std::thread        m_thread;
	bool               m_running = true;

	RTPTCPTransmitter* m_rtpTcpTransmitter = nullptr;
};

#endif

