#include "TcpServer.h"

static int TcpDataThread(void* param)
{
	assert(param);
	TcpServer* client = (TcpServer*)param;
	client->TcpDataWorker();
	return 0;
}

TcpServer::TcpServer(TcpDataCallBack func, void* userdata)
	: m_func(func)
	, m_userdata(userdata)
	, m_socketS(INVALID_SOCKET)
	, m_sockAddrS({ 0 })
	, m_socketC(INVALID_SOCKET)
	, m_sockAddrC({ 0 })
	, m_running(false)
{
	// 初始化套接字 
	WSADATA wsaData;
	if (0 == WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		m_running = true;
		m_thread = std::thread(TcpDataThread, this);
		m_recvBuf = std::shared_ptr<char>(new char[TCP_SERVER_DATA_SIZE], std::default_delete<char[]>());
	}
}

TcpServer::~TcpServer()
{
	TcpDestroy();
	WSACleanup();
}

int TcpServer::TcpCreate()
{
	TcpClose_();

	m_socketS = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == m_socketS)
	{
		return -1;
	}

	return 0;
}

int TcpServer::TcpBind(int port)
{
	if (INVALID_SOCKET == m_socketS || port <= 0)
		return -1;

	m_sockAddrS.sin_family = AF_INET;
	m_sockAddrS.sin_port = htons(port);//转网络字节序
	m_sockAddrS.sin_addr.S_un.S_addr = INADDR_ANY;//任何ip
	return bind(m_socketS, (const sockaddr*)&m_sockAddrS, sizeof(m_sockAddrS));
}

int TcpServer::TcpListen(int num)
{
	if (INVALID_SOCKET == m_socketS || num <= 0)
		return -1;

	return listen(m_socketS, num);
}

int TcpServer::TcpAccept()
{
	return TcpAccept_();
}

void TcpServer::TcpDestroy()
{
	m_running = false;
	if (m_thread.joinable())
		m_thread.join();

	TcpClose_();
}

SOCKET TcpServer::GetClientSocket() const
{
	return m_socketC;
}

int TcpServer::TcpSetNoBlock_(bool onoff)
{
	if (INVALID_SOCKET == m_socketC)
		return -1;

	DWORD val = (onoff ? 1 : 0);
	return ioctlsocket(m_socketC, FIONBIO, &val);
}

int TcpServer::TcpTimeout_(int seconds)
{
	if (INVALID_SOCKET == m_socketC || seconds <= 0)
		return -1;

	struct timeval tv;
	tv.tv_sec = seconds;  //秒
	tv.tv_usec = 0;       //微秒
	return setsockopt(m_socketC, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
}

int TcpServer::TcpAccept_()
{
	if (INVALID_SOCKET == m_socketS)
		return -1;

	int len = sizeof(m_sockAddrC);
	m_socketC = accept(m_socketS, (sockaddr*)&m_sockAddrC, &len);
	if (INVALID_SOCKET == m_socketC)
		return -1;

	return 0;
}

int TcpServer::TcpRecv_(void* buf, int len)
{
	if (INVALID_SOCKET == m_socketC)
		return -1;

	return recv(m_socketC, (char*)buf, len, 0);
}

void TcpServer::TcpClose_()
{
	if (INVALID_SOCKET != m_socketS)
	{
		closesocket(m_socketS);
		m_socketS = INVALID_SOCKET;
	}

	if (INVALID_SOCKET != m_socketC)
	{
		closesocket(m_socketC);
		m_socketC = INVALID_SOCKET;
	}
}

int TcpServer::BeginReceive_(void*& buf, int& len)
{
	if (RECV_HEAD == m_status)
	{
		buf = m_recvBuf.get();
		len = RTP_HEAD_SIZE;
	}
	else if (RECV_BODY == m_status)
	{
		buf = m_recvBuf.get() + RTP_HEAD_SIZE;
		len = (m_recvBuf.get()[0] << 8) + (m_recvBuf.get()[1] & 0xff);
	}

	return 0;
}

int TcpServer::EndReceive_()
{
	if (RECV_HEAD == m_status)
	{
		m_status = RECV_BODY;
	}
	else if (RECV_BODY == m_status)
	{
		m_recvLen = (m_recvBuf.get()[0] << 8) + (m_recvBuf.get()[1] & 0xff);
		if (m_func)
			m_func(m_recvBuf.get() + RTP_HEAD_SIZE, m_recvLen, m_userdata);
		m_status = RECV_HEAD;

		memset(m_recvBuf.get(), 0, TCP_SERVER_DATA_SIZE);
	}

	return 0;
}

void TcpServer::TcpDataWorker()
{
	//void* recvBuf = nullptr;
	auto recvBuf = std::shared_ptr<char>(new char[TCP_SERVER_DATA_SIZE], std::default_delete<char[]>());
	int recvLen = 0;

	bool bAccept = false;
	while (m_running)
	{
		if (!bAccept)
		{
			if (0 == TcpAccept_())
			{
				bAccept = true;
				TcpSetNoBlock_(true);
				TcpTimeout_(2);
			}

			continue;
		}

		//BeginReceive_(recvBuf, recvLen);
		
		if (RECV_HEAD == m_status)
		{
			recvLen = RTP_HEAD_SIZE;
		}
		else if (RECV_BODY == m_status)
		{
			//buf = m_recvBuf.get() + RTP_HEAD_SIZE;
			//recvLen = (m_recvBuf.get()[0] << 8) + (m_recvBuf.get()[1] & 0xff);
		}

		int len = TcpRecv_(recvBuf.get(), recvLen);
		if (len <= 0)
		{
			continue;
		}

		if (RECV_HEAD == m_status)
		{
			m_status = RECV_BODY;
			recvLen = (recvBuf.get()[0] << 8) + (recvBuf.get()[1] & 0xff);
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			continue;
		}
		else if (RECV_BODY == m_status)
		{
			recvLen = len;
			if (m_func)
				m_func(recvBuf.get(), recvLen, m_userdata);
			m_status = RECV_HEAD;
		}

		//EndReceive_();
		//Sleep(2);
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}