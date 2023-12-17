// rtsp收流类
#ifndef _RTSP_TCP_STREAM_RECEIVER_H_
#define _RTSP_TCP_STREAM_RECEIVER_H_

#include "ZDStreamReceiverInterface.h"
#include "ZDTcpClient.h"
#include "RtspCommand.h"
#include <string>
#include <thread>
#include <assert.h>

#define  STREAM_DATA_SIZE  (512*1024)

class CRtspTcpStreamReceiver : public IZDStreamReceiver
{
public:
	CRtspTcpStreamReceiver(const char* url, StreamDataCallBack func, void* userParam);
	virtual ~CRtspTcpStreamReceiver();

	void RtspWorker();

public:
	virtual void DeleteThis() override;

	// 开始收流
	virtual int Start(int streamType) override;

	// 停止收流
	virtual int Stop() override;

private:
	int InitRtspSession_();
	void UnInitRtspSession_();
	int ParseUrl_(const std::string& url, std::string& username, std::string& userpasswd, std::string& ip, int& port);

private:
	std::string   m_rtspUrl;

	StreamDataCallBack  m_func;
	void*         m_user;

	RtspCommandPtr m_command;
	ZDTcpClientPtr m_tcpClient;

	int           m_status = -1;
	std::thread   m_thread;
	bool          m_running = true;
	bool          m_isVideo = false;
};

#endif

