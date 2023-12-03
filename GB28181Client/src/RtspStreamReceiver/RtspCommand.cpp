#include "RtspCommand.h"

static int SendCallback(void* param, const char* uri, const void* req, size_t bytes)
{
	assert(param);
	CRtspCommand* command = (CRtspCommand*)param;
	return command->HandleMethodSend(uri, req, bytes);
}

static int RtpPortCallcback(void* param, int media, const char* source, unsigned short rtp[2], char* ip, int len)
{
	assert(param);
	CRtspCommand* command = (CRtspCommand*)param;
	return command->HandleRtpPort(media, source, rtp, ip, len);
}

static int AnnounceCallback(void* param)
{
	assert(param);
	CRtspCommand* command = (CRtspCommand*)param;
	return command->HandleMethodAnnounce();
}

static int DescribeCallback(void* param, const char* sdp, int len)
{
	assert(param);
	CRtspCommand* command = (CRtspCommand*)param;
	return command->HandleMethodDescribe(sdp, len);
}

static int SetupCallback(void* param, int timeout, int64_t duration)
{
	assert(param);
	CRtspCommand* command = (CRtspCommand*)param;
	return command->HandleMethodSetup(timeout, duration);
}

static int PlayCallback(void* param,
	int media,
	const uint64_t* nptbegin,
	const uint64_t* nptend,
	const double* scale,
	const struct rtsp_rtp_info_t* rtpinfo,
	int count)
{
	assert(param);
	CRtspCommand* command = (CRtspCommand*)param;
	return command->HandleMethodPlay();
}

static int PauseCallback(void* param)
{
	return 0;
}

static int TeardownCallback(void* param)
{
	return 0;
}

static void RtpDataCallback(void* param, uint8_t channel, const void* data, uint16_t bytes)
{
	assert(param);
	CRtspCommand* command = (CRtspCommand*)param;
	command->InputRtpData(channel, data, bytes);
}

CRtspCommand::CRtspCommand(int transport, TcpClientPtr client, StreamDataCallBack func, void* userParam)
	: m_transport(transport)
	, m_tcpClient(client)
	, m_func(func)
	, m_user(userParam)
{
	m_timer = std::make_shared<ZDTimer>(10, 1000);
	m_timer->Start();
}

CRtspCommand::~CRtspCommand()
{
	if (m_timer.get())
	{
		m_timer->Stop();
		m_timer.reset();
	}
}

bool CRtspCommand::CreateRtspClient(const std::string& uri, const std::string& username, const std::string& userpasswd)
{
	if (uri.empty() || username.empty() || userpasswd.empty())
		return false;

	rtsp_client_handler_t handler;
	handler.send = SendCallback;
	handler.rtpport = RtpPortCallcback;
	handler.onannounce = AnnounceCallback;
	handler.ondescribe = DescribeCallback;
	handler.onsetup = SetupCallback;
	handler.onplay = PlayCallback;
	handler.onpause = PauseCallback;
	handler.onteardown = TeardownCallback;
	handler.onrtp = RtpDataCallback;

	m_rtsp = rtsp_client_create(uri.c_str(), username.c_str(), userpasswd.c_str(), &handler, this);
	if (!m_rtsp)
		return false;

	return true;
}

void CRtspCommand::DestroyRtspClient()
{
	if (!m_rtsp)
		return;

	rtsp_client_destroy(m_rtsp);
}

int CRtspCommand::SendDescribe()
{
	if (!m_rtsp)
		return -1;

	return rtsp_client_describe(m_rtsp);
}

int CRtspCommand::SendTeardown()
{
	if (!m_rtsp)
		return -1;

	return rtsp_client_teardown(m_rtsp);
}

int CRtspCommand::InputData(void* data, size_t bytes)
{
	if (!m_rtsp || !data || bytes < 0)
		return -1;

	return rtsp_client_input(m_rtsp, data, bytes);
}

int CRtspCommand::InputRtpData(uint8_t channel, const void* data, uint16_t bytes)
{
	if (!m_tcpReceiver.get())
		return -1;

	m_tcpReceiver->InputRtpData(data, bytes);
	return 0;
}

int CRtspCommand::HandleMethodSend(const char* uri, const void* req, size_t bytes)
{
	if (!m_tcpClient.get())
		return -1;

	return m_tcpClient->TcpSend((void*)req, bytes, 0, 2000);;
}

int CRtspCommand::HandleMethodAnnounce()
{
	return 0;
}

int CRtspCommand::HandleMethodDescribe(const char* sdp, int len)
{
	if (!sdp || len <= 0 || !m_rtsp)
		return -1;

	return rtsp_client_setup((rtsp_client_t*)m_rtsp, sdp, len);;
}

int CRtspCommand::HandleMethodSetup(int timeout, int64_t duration)
{
	if (!m_rtsp)
		return -1;

	uint64_t npt = 0;
	if (0 != rtsp_client_play((rtsp_client_t*)m_rtsp, &npt, nullptr))
	{
		return -1;
	}

	m_tcpReceiver = std::make_shared<CRtspTcpDataReceiver>(false, m_func, m_user);
	if (!m_tcpReceiver.get())
		return -1;

	int count = rtsp_client_media_count((rtsp_client_t*)m_rtsp);
	for (int i = 0; i < count; ++i)
	{
		const struct rtsp_header_transport_t* transport = rtsp_client_get_media_transport((rtsp_client_t*)m_rtsp, i);
		const char* encoding = rtsp_client_get_media_encoding((rtsp_client_t*)m_rtsp, i);
		int payload = rtsp_client_get_media_payload((rtsp_client_t*)m_rtsp, i);
		if (RTSP_TRANSPORT_RTP_UDP == transport->transport)
		{
		}
		else if (RTSP_TRANSPORT_RTP_TCP == transport->transport)
		{
			m_tcpReceiver->Start(transport->interleaved1, transport->interleaved2, payload, encoding);
		}
		else
		{
			assert(0);
		}
	}

	return 0;
}

int CRtspCommand::HandleMethodPlay()
{
	if (m_timer.get() && m_rtsp && RTSP_TRANSPORT_RTP_TCP == m_transport)
	{
		if (!m_bStartTimer)
		{
			m_timer->AddTask(10000, [this]() {
				int count = rtsp_client_media_count((rtsp_client_t*)m_rtsp);
				if (count > 0)
					rtsp_client_get_parameter(m_rtsp, 0, NULL);
				});

			m_bStartTimer = true;
		}
		
	}

	return 0;
}

int CRtspCommand::HandleRtpPort(int media, const char* source, unsigned short rtp[2], char* ip, int len)
{
	switch (m_transport)
	{
	case RTSP_TRANSPORT_RTP_UDP:
		break;
	case RTSP_TRANSPORT_RTP_TCP:
		rtp[0] = (unsigned short)(2 * media);
		rtp[1] = (unsigned short)(2 * media + 1);
		break;

	default:
		assert(0);
		return -1;
	}

	return m_transport;
}
