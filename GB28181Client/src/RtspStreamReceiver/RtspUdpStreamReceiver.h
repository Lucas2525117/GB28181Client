#ifndef _RTSP_UDP_STREAM_RECEIVER_H_
#define _RTSP_UDP_STREAM_RECEIVER_H_

#include "ZDStreamReceiverInterface.h"
#include "ZDTcpClient.h"
#include "RtspCommand.h"
#include <string>
#include <thread>
#include <assert.h>

#define  UDP_STREAM_DATA_SIZE  (512*1024)

class CRtspUdpStreamReceiver : public IZDStreamReceiver
{
public:
	CRtspUdpStreamReceiver(const char* url, StreamDataCallBack func, void* userParam);
	virtual ~CRtspUdpStreamReceiver();

	void RtspUdpWorker();

public:
	virtual void DeleteThis() override;

	// ��ʼ����
	virtual int Start(int streamType) override;

	// ֹͣ����
	virtual int Stop() override;

private:
	int InitRtspSession_();
	void UnInitRtspSession_();
	int ParseUrl_(const std::string& url, std::string& username, std::string& userpasswd, std::string& ip, int& port);

private:
	std::string   m_rtspUrl;

	StreamDataCallBack  m_func;
	void*         m_user;

	int           m_status = -1;
	std::thread   m_thread;
	bool          m_running = true;
	bool          m_isVideo = false;

	ZDTcpClientPtr m_tcpClient;
	RtspCommandPtr m_command;
};

#endif

