#include "RtspUdpStreamReceiver.h"

int RtspUdpThread(void* param)
{
	assert(param);
	CRtspUdpStreamReceiver* receiver = (CRtspUdpStreamReceiver*)param;
	receiver->RtspUdpWorker();
	return 0;
}

CRtspUdpStreamReceiver::CRtspUdpStreamReceiver(const char* url, StreamDataCallBack func, void* userParam)
	: m_rtspUrl(url)
	, m_func(func)
	, m_user(userParam)
{
}

CRtspUdpStreamReceiver::~CRtspUdpStreamReceiver()
{
	Stop();
}

void CRtspUdpStreamReceiver::DeleteThis()
{
	delete this;
}

int CRtspUdpStreamReceiver::Start(int streamType)
{
	m_status = InitRtspSession_();
	if (0 != m_status)
	{
		return m_status;
	}

	m_thread = std::thread(RtspUdpThread, this);
	return 0;
}

int CRtspUdpStreamReceiver::Stop()
{
	m_running = false;
	if (m_thread.joinable())
		m_thread.join();

	return 0;
}

void CRtspUdpStreamReceiver::RtspUdpWorker()
{
	std::shared_ptr<char> dataPacket(new char[UDP_STREAM_DATA_SIZE], std::default_delete<char[]>());
	memset(dataPacket.get(), 0x00, UDP_STREAM_DATA_SIZE);
	int recvLen = 0;
	int recvTimes = 0;
	bool bFirstRecv = false;

	while (m_running)
	{
		recvLen = m_tcpClient->TcpRecv(dataPacket.get(), UDP_STREAM_DATA_SIZE);
		if (recvLen <= 0)       // 数据接收失败
		{
			if (!bFirstRecv)
			{
				recvTimes++;
				if (recvTimes >= 100)
				{
					m_status = -1;
					break;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
				continue;
			}
			else              // 非第一次数据接收失败,等待
			{
				if (m_isVideo)
				{
					// rtsp播放录像流, 1s内收不到录像认为录像结束
					recvTimes++;
					if (recvTimes >= 100)
					{
						m_status = 1;   // vod end
						break;
					}
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
				}

				continue;
			}
		}

		if (0 != m_command->InputData(dataPacket.get(), recvLen))
		{
			break;
		}

		memset(dataPacket.get(), 0x00, UDP_STREAM_DATA_SIZE);
		bFirstRecv = true;
	}
}

int CRtspUdpStreamReceiver::InitRtspSession_()
{
	if (m_rtspUrl.empty())
		return -1;

	std::string username, userpasswd, ip;
	int port = -1;
	if (0 != ParseUrl_(m_rtspUrl, username, userpasswd, ip, port))
	{
		return -1;
	}

	do 
	{
		m_tcpClient = std::make_shared<ZDTcpClient>(nullptr, this);
		if (!m_tcpClient.get()
			|| 0 != m_tcpClient->TcpCreate()
			|| 0 != m_tcpClient->TcpConnect(ip.c_str(), port)
			|| 0 != m_tcpClient->TcpSetNoBlock(true)
			|| 0 != m_tcpClient->TcpRecvTimeout(10))
			break;

		m_command = std::make_shared<CRtspCommand>(RTSP_TRANSPORT_RTP_UDP, m_tcpClient, m_func, m_user);
		if (!m_command.get())
			break;

		if (!m_command->CreateRtspClient(m_rtspUrl, username, userpasswd))
			break;

		int ret = m_command->SendDescribe();
		if (0 != ret)
			break;

		return 0;
	} while (0);

	UnInitRtspSession_();
	return -1;
}

void CRtspUdpStreamReceiver::UnInitRtspSession_()
{
	if (m_command.get())
	{
		m_command->DestroyRtspClient();
		m_command.reset();
	}

	if (m_tcpClient.get())
	{
		m_tcpClient->TcpDestroy();
		m_tcpClient.reset();
	}
}

int CRtspUdpStreamReceiver::ParseUrl_(const std::string& url, std::string& username, std::string& userpasswd, std::string& ip, int& port)
{
	std::string::size_type pos_0 = url.find("rtsp://");
	if (std::string::npos == pos_0)
		return -1;

	std::string::size_type pos_4 = 0;
	if ((int)url.find("/Streaming/Channels") > 0)    // hik预览
	{
		pos_4 = url.find("/Streaming/Channels");
	}
	else if ((int)url.find("/Streaming/tracks") > 0) // hik回放1
	{
		m_isVideo = true;
		pos_4 = url.find("/Streaming/tracks");
	}
	else if ((int)url.find("/Streaming/Tracks") > 0) // hik回放2
	{
		m_isVideo = true;
		pos_4 = url.find("/Streaming/Tracks");
	}
	else if ((int)url.find("/cam/realmonitor") > 0)  // dahua预览
	{
		pos_4 = url.find("/cam/realmonitor");
	}
	else if ((int)url.find("/cam/playback") > 0)     // dahua回放
	{
		m_isVideo = true;
		pos_4 = url.find("/cam/playback");
	}
	else if ((int)url.find("/unicast") > 0)          // yushi预览(NVR)
	{
		pos_4 = url.find("/unicast");
	}
	else if ((int)url.find("/media") > 0)            // 宇视预览(IPC1---for onvif)
	{
		// 方式1: rtsp://admin:Lmtyf!123@188.188.1.50:554/media1/video1
		// 方式2: rtsp://admin:Lmtyf!123@188.188.1.50/media1/video1
		pos_4 = url.find("/media");
	}
	else if ((int)url.find("/video") > 0)            // 宇视预览(IPC2)
	{
		//  rtsp://admin:Lmtyf!123@188.188.1.50:554/media1/video1
		//  rtsp://admin:Lmtyf!123@188.188.1.50:554/video1
		if ((int)url.find("/media") > 0)
		{
			pos_4 = url.find("/media");
		}
		else
		{
			pos_4 = url.find("/video");
		}
	}
	else if ((int)url.find("/replay") > 0)           // yushi回放
	{
		m_isVideo = true;
		pos_4 = url.find("/ch");
	}
	else
	{
		return -1;
	}

	std::string tmpUrl = url.substr(pos_0 + 7, pos_4 - 7);
	std::string::size_type pos_1 = tmpUrl.rfind("@");
	if (std::string::npos == pos_1)
		return -1;

	std::string  name_passwd = tmpUrl.substr(0, pos_1);
	std::string::size_type pos_2 = name_passwd.find(":");
	if (std::string::npos == pos_2)
		return -1;

	username = name_passwd.substr(0, pos_2);
	userpasswd = name_passwd.substr(pos_2 + 1, name_passwd.length());

	std::string ip_port = tmpUrl.substr(pos_1 + 1, tmpUrl.length());
	std::string::size_type pos_3 = ip_port.find(":");
	if (std::string::npos == pos_3)
	{
		ip = ip_port;
		port = 554;
		return 0;
	}

	ip = ip_port.substr(0, pos_3);
	port = atoi((ip_port.substr(pos_3 + 1, ip_port.length())).c_str());
	return 0;
}
