// rtsp������
#ifndef _RTSP_STREAM_RECEIVER_H_
#define _RTSP_STREAM_RECEIVER_H_

#include "RtspStreamReceiverInterface.h"
#include "TcpClient.h"
#include "RtspCommand.h"
#include <string>
#include <thread>
#include <assert.h>

#define  STREAM_DATA_SIZE  (512*1024)

class CRtspStreamReceiver : public IRtspStreamReceiver
{
public:
	CRtspStreamReceiver(const char* url, StreamDataCallBack func, void* userParam);
	virtual ~CRtspStreamReceiver();

	void RtspWorker();

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

	RtspCommandPtr m_command;
	TcpClientPtr   m_tcpClient;

	int           m_status = -1;
	std::thread   m_thread;
	bool          m_running = true;
	bool          m_isVideo = false;
};

#endif

