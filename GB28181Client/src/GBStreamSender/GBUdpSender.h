#ifndef _GB_UDP_SENDER_H_
#define _GB_UDP_SENDER_H_

#include "StreamSenderInterface.h"
#include "rtpsession.h"
#include "Data2PS.h"
#include <thread>
#include <string>

using namespace jrtplib;

class CGBUdpSender : public IStreamSender, public RTPSession
{
public:
	CGBUdpSender(const char* gbUrl);
	virtual ~CGBUdpSender();

	void InputPSData(int64_t pts, void* _data, int _len);

	void Data2PSWorker();

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
	virtual void OnRTCPCompoundPacket(RTCPCompoundPacket* pack, const RTPTime& receivetime, const RTPAddress* senderaddress) override;
	virtual void OnSendRTCPCompoundPacket(RTCPCompoundPacket* pack) override;
	virtual void OnTimeout(RTPSourceData* srcdat) override;
	int ParseUrl_(const std::string& gburl);
	int InitRtpSession_();

private:
	CData2PS*         m_pspacker = nullptr;

	std::thread       m_thread;
	bool              m_running = true;

	std::string       m_gbUrl;
	std::string       m_localIP = "";
	int64_t           m_pts = 0;
	int64_t           m_lastRRTime = 0;
	int               m_status = 0;
	int               m_localPort = 0;
	int               m_payload = 96;
	int               m_ssrc = 0;
};

#endif

