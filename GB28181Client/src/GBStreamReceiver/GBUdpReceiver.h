#ifndef _GB_UDP_RECEIVER_H_
#define _GB_UDP_RECEIVER_H_

#include "StreamReceiverInterface.h"
#include "rtpsession.h"
#include "GBPublic.h"
#include "PSParse.h"
#include "G711AParse.h"
#include <winsock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <assert.h>

using namespace jrtplib;

struct rtp_packet
{
	uint32_t  pts;
	uint16_t  seq;
	int       len;
	bool      mark;
	uint8_t*  data;
};

#define ARRAY_SIZE           (16)

class CGBUdpStreamReceiver : public IStreamReceiver, public RTPSession
{
public:
	CGBUdpStreamReceiver(const char* gbUrl, GBDataCallBack func, void* userParam);
	virtual ~CGBUdpStreamReceiver();

public:
	virtual void DeleteThis() override;

	// 开始收流
	virtual int Start(int streamType) override;

	// 停止收流
	virtual int Stop() override;

public:
	void VideoDataWorker();
	void AudioDataWorker();

private:
	int ParseUrl_(const std::string& gburl);
	int InitRtpSession_();
	int UdpConnect_();
	int PackData_(struct rtp_packet* packet);
	int ProcessData_(uint32_t pts);
	int ProcessCachePackets_(struct rtp_packet* packet);

	int PackAudioData_(void* data, int len);

	SOCKET UDPCreate_();
	bool UDPBind_(SOCKET sock, int port);
	void UDPClose_(SOCKET sock);

private:
	std::string       m_gbUrl;
	std::string       m_localIP;
	int               m_localPort = 0;
	bool              m_running = true;

	unsigned char*    m_recvBuf = nullptr;
	int               m_bufSize = 0;
	int               m_dataLen = 0;

	GBDataCallBack    m_func;
	void*             m_user = nullptr;

	struct rtp_packet* m_packets[ARRAY_SIZE];
	unsigned char     m_payload;
	int               m_lastSeq;
	uint32_t          m_lastPts;

	uint64_t          m_baseTime = 0;
	CPSParse*         m_parse = nullptr;
	CG711AParse*      m_G711AParse = nullptr;
	bool              m_lost = false;
	bool              m_needIframe = false;

	std::thread       m_thread;
	int               m_streamType = 0;
};

#endif

