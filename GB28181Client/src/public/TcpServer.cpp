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

void TcpServer::TcpDestroy()
{
	m_running = false;
	if (m_thread.joinable())
		m_thread.join();

	TcpClose_();
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

void TcpServer::TcpDataWorker()
{
	std::shared_ptr<char> recvBuf(new char[TCP_DATA_SIZE], std::default_delete<char[]>());
	memset(recvBuf.get(), 0x00, TCP_DATA_SIZE);

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
		
		int recvLen = TcpRecv_(recvBuf.get(), TCP_DATA_SIZE);
		if (recvLen <= 0)
		{
			continue;
		}

		if (m_func)
			m_func(recvBuf.get(), recvLen, m_userdata);

		memset(recvBuf.get(), 0x00, TCP_DATA_SIZE);
	}
}