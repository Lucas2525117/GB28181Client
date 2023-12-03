#ifndef _RTSP_COMMAND_H_
#define _RTSP_COMMAND_H_

#include "StreamPublic.h"
#include "rtsp-client-internal.h"
#include "rtsp-client.h"
#include "RtspTcpDataReceiver.h"
#include "TcpClient.h"
#include "ZDTimer.h"
#include <string>
#include <memory>
#include <assert.h>

class CRtspCommand
{
public:
	CRtspCommand(int transport, TcpClientPtr client, StreamDataCallBack func, void* userParam);  // transport 1:udp 2:tcp
	~CRtspCommand();

	bool CreateRtspClient(const std::string& uri, const std::string& username, const std::string& userpasswd);

	void DestroyRtspClient();

	int SendDescribe();

	int SendTeardown();

	int InputData(void* data, size_t bytes);

	int InputRtpData(uint8_t channel, const void* data, uint16_t bytes);

public:
	int HandleMethodSend(const char* uri, const void* req, size_t bytes);
	int HandleMethodAnnounce();
	int HandleMethodDescribe(const char* sdp, int len);
	int HandleMethodSetup(int timeout, int64_t duration);
	int HandleMethodPlay();
	int HandleRtpPort(int media, const char* source, unsigned short rtp[2], char* ip, int len);

private:
	int                    m_transport;
	TcpClientPtr           m_tcpClient;
	StreamDataCallBack     m_func;
	void*                  m_user;

	rtsp_client_t*         m_rtsp = nullptr;
	RtspTcpDataReceiverPtr m_tcpReceiver;
	std::shared_ptr<ZDTimer> m_timer;
	bool                   m_bStartTimer = false;
};

typedef std::shared_ptr<CRtspCommand> RtspCommandPtr;

#endif

