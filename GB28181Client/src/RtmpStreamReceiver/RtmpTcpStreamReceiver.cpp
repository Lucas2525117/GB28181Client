#include "RtmpTcpStreamReceiver.h"

int RtmpThread(void* param)
{
	assert(param);
	CRtmpTcpStreamReceiver* receiver = (CRtmpTcpStreamReceiver*)param;
	receiver->RtmpWorker();
	return 0;
}

CRtmpTcpStreamReceiver::CRtmpTcpStreamReceiver(const char* url, StreamDataCallBack func, void* userParam)
	: m_rtmpUrl(url)
	, m_func(func)
	, m_user(userParam)
{
}

CRtmpTcpStreamReceiver::~CRtmpTcpStreamReceiver()
{
	Stop();
}

void CRtmpTcpStreamReceiver::DeleteThis()
{
	delete this;
}

int CRtmpTcpStreamReceiver::Start(int streamType)
{
	if (0 == m_status)
		return 0;

	m_status = InitRtmpSession_();
	if (0 != m_status)
	{
		return m_status;
	}

	m_thread = std::thread(RtmpThread, this);
	return m_status;
}

int CRtmpTcpStreamReceiver::Stop()
{
	m_running = false;
	if (m_thread.joinable())
		m_thread.join();

	UnInitRtmpSession_();
	return 0;
}

int CRtmpTcpStreamReceiver::InitRtmpSession_()
{
	if (0 != ParseUrl_(m_rtmpUrl))
	{
		return -1;
	}

	do
	{
		m_tcpClient = std::make_shared<ZDTcpClient>(nullptr, this);
		if (!m_tcpClient.get()
			|| 0 != m_tcpClient->TcpCreate()
			|| 0 != m_tcpClient->TcpConnect("", 0)
			|| 0 != m_tcpClient->TcpSetNoBlock(false)
			|| 0 != m_tcpClient->TcpRecvTimeout(5))
			break;

		m_command = std::make_shared<CRtmpCommand>(m_tcpClient, m_func, m_user);
		if (!m_command.get())
			break;

		if (0 != m_command->Create("", "", ""))
			break;

		if (0 != m_command->Start(1))
			break;

		return 0;
	} while (0);

	UnInitRtmpSession_();
	return -1;
}

void CRtmpTcpStreamReceiver::UnInitRtmpSession_()
{
	if (m_command.get())
	{
		m_command->Destroy();
		m_command.reset();
	}

	if (m_tcpClient.get())
	{
		m_tcpClient->TcpDestroy();
		m_tcpClient.reset();
	}
}

int CRtmpTcpStreamReceiver::ParseUrl_(const std::string& url)
{
	return 0;
}

void CRtmpTcpStreamReceiver::RtmpWorker()
{
	std::shared_ptr<char> dataPacket(new char[RTMP_STREAM_DATA_SIZE], std::default_delete<char[]>());
	memset(dataPacket.get(), 0x00, RTMP_STREAM_DATA_SIZE);
	int recvLen = 0;
	int recvTimes = 0;
	bool bFirstRecv = false;

	while (m_running)
	{
		recvLen = m_tcpClient->TcpRecv(dataPacket.get(), RTMP_STREAM_DATA_SIZE);
		if (recvLen < 0) 
		{
			if (!bFirstRecv)
			{
				recvTimes++;
				if (recvTimes >= 100)
				{
					m_status = -1;
					break;
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				continue;
			}
		}

		if (0 != m_command->InputData(dataPacket.get(), recvLen))
		{
			break;
		}

		memset(dataPacket.get(), 0x00, RTMP_STREAM_DATA_SIZE);
		bFirstRecv = true;
	}
}