#ifndef _RTSP_UDPDATA_H_
#define _RTSP_UDPDATA_H_

#include "StreamPublic.h"
#include "StreamDefine.h"
#include "RtspDefine.h"
#include "rtp-profile.h"
#include "rtp-demuxer.h"
#include "rtp-payload.h"
#include "rtcp-header.h"
#include "sys/sock.h"
#include "sys/pollfd.h"
#include "XXXInpack.h"
#include "H264Inpack.h"
#include "H265Inpack.h"
#include <WinSock2.h>
#include <memory>
#include <string>
#include <thread>

class CRtspUdpData
{
public:
	CRtspUdpData(bool isVideo, StreamDataCallBack func, void* userParam);
	~CRtspUdpData();

	int Start(int count, socket_t rtp[2], const char* peer, int peerport[2], int payload, const char* encoding);

	void Stop();

public:
	void UdpDataWorker();

	void Package(const void* packet, int bytes, unsigned int timestamp, int flags);

private:
	int CreatePacker_(int payload, const std::string& encoding);
	int RtpRead_(socket_t s);
	int RtcpRead_(socket_t s);

private:
	bool m_isVideo = false;
	StreamDataCallBack m_func;
	void* m_user;

	rtp_demuxer_t* m_demuxer = nullptr;
	std::shared_ptr<XXX2InPack> m_xxx2Inpack;

	std::thread  m_thread;
	bool         m_running = true;
	std::string  m_encoding = "";
	int          m_payload = 0;
	socket_t     m_socket[2];
	char         m_rtpBuffer[64 * 1024];
	char         m_rtcpBuffer[32 * 1024];
	struct sockaddr_storage m_sockaddrStorage[2];

};

typedef std::shared_ptr<CRtspUdpData> RtspUdpDataPtr;

#endif

