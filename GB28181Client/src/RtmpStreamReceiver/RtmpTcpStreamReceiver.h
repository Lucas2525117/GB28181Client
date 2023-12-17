// rtmp收流类
#ifndef _RTMP_TCP_STREAM_RECEIVER_H_
#define _RTMP_TCP_STREAM_RECEIVER_H_

#include "ZDStreamReceiverInterface.h"
#include "ZDTcpClient.h"
#include "RtmpCommand.h"
#include <string>
#include <thread>
#include <assert.h>

#define  RTMP_STREAM_DATA_SIZE  (512*1024)

class CRtmpTcpStreamReceiver : public IZDStreamReceiver
{
public:
	CRtmpTcpStreamReceiver(const char* url, StreamDataCallBack func, void* userParam);
	virtual ~CRtmpTcpStreamReceiver();

	void RtmpWorker();

public:
	virtual void DeleteThis() override;

	// 开始收流
	virtual int Start(int streamType) override;

	// 停止收流
	virtual int Stop() override;

private:
	int InitRtmpSession_();
	void UnInitRtmpSession_();
	int ParseUrl_(const std::string& url);

private:
	std::string   m_rtmpUrl;

	StreamDataCallBack  m_func;
	void* m_user;

	ZDTcpClientPtr m_tcpClient;
	RtmpCommandPtr m_command;

	int           m_status = -1;
	std::thread   m_thread;
	bool          m_running = true;
};

#endif