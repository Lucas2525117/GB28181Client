#ifndef _GB_TCP_CLIENT_RECEIVER_H_
#define _GB_TCP_CLIENT_RECEIVER_H_

#include "StreamReceiverInterface.h"
#include "TcpClient.h"
#include "Rtp2PS.h"
#include "PSParse.h"
#include "rtpsession.h"
#include "rtpsessionparams.h"
#include "rtptcptransmitter.h"
#include "rtptcpaddress.h"
#include "rtppacket.h"
#include "rtpsources.h"
#include "rtpsourcedata.h"
#include "rtpaddress.h"
#include "rtpipv4address.h"
#include <string>
#include <thread>

using namespace jrtplib;

struct rtp_packet_tcp_active
{
	uint32_t  pts;
	uint16_t  seq;
	int       len;
	bool      mark;
	uint8_t* data;
};

class CGBTcpClientStreamReceiver : public IStreamReceiver, public RTPSession
{
public:
	CGBTcpClientStreamReceiver(const char* gbUrl, StreamDataCallBack func, void* userParam);
	virtual ~CGBTcpClientStreamReceiver();

	void InputTcpData(void* data, int len);
	void InputPSData(void* data, int len);

public:
	virtual void DeleteThis() override;

	// 开始收流
	virtual int Start(int streamType) override;

	// 停止收流
	virtual int Stop() override;

public:
	void TcpDataWorker();

private:
	int ParseUrl_(const std::string& gburl);
	int InitRtpSession_();

private:
	StreamDataCallBack    m_func;
	void*             m_userdata;

	std::string       m_gbUrl;
	std::string       m_localIP;
	int               m_localPort = 0;

	TcpClientPtr      m_tcpClient;
	CRtp2Ps           m_rtp2PS;
	std::shared_ptr<CPSParse> m_parse;

	unsigned char     m_payload = 0;
	int               m_lastSeq = 0;
	std::thread       m_thread;
	bool              m_running = true;

	RTPTCPTransmitter* m_rtpTcpTransmitter = nullptr;
};

#endif  // _GB_TCP_CLIENT_RECEIVER_H_

