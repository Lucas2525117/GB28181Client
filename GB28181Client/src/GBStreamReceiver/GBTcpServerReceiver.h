#ifndef _GB_TCP_SERVER_RECEIVER_H_
#define _GB_TCP_SERVER_RECEIVER_H_

#include "StreamReceiverInterface.h"
#include "rtpsession.h"
#include "TcpServer.h"
#include "Rtp2PS.h"
#include "PSParse.h"
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

struct rtp_packet_tcp
{
	uint32_t  pts;
	uint16_t  seq;
	int       len;
	bool      mark;
	uint8_t* data;
};

class CGBTcpServerStreamReceiver : public IStreamReceiver, public RTPSession
{
public:
	CGBTcpServerStreamReceiver(const char* gbUrl, GBDataCallBack func, void* userParam);
	virtual ~CGBTcpServerStreamReceiver();

	void InputTcpData(void* data, int len);

	void InputRtpData(void* data, int len);

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
	void PackData_(void* data, int len);

private:
	GBDataCallBack    m_func;
	void*             m_userdata;
	std::thread       m_thread;
	bool              m_running = true;

	std::string       m_gbUrl;
	std::string       m_localIP;
	int               m_localPort = 0;
	unsigned char     m_payload = 0;
	int               m_lastSeq = 0;

	TcpServerPtr      m_tcpServer;
	CRtp2Ps           m_rtp2PS;
	std::shared_ptr<CPSParse> m_parse;

	RTPTCPTransmitter* m_rtpTcpTransmitter = nullptr;
};

#endif  // _GB_TCP_SERVER_RECEIVER_H_

