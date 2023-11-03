#include "TcpClient.h"

static int TcpDataThread(void* param)
{
	assert(param);
	TcpClient* client = (TcpClient*)param;
	client->TcpDataWorker();
	return 0;
}

TcpClient::TcpClient(TcpDataCallBack func, void* userdata)
	: m_func(func)
	, m_userdata(userdata)
	, m_socket(INVALID_SOCKET)
	, m_sockAddr({ 0 })
	, m_running(false)
{
	// ³õÊ¼»¯Ì×½Ó×Ö 
	WSADATA wsaData;
	if (0 == WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		m_running = true;
		m_thread = std::thread(TcpDataThread, this);
	}
}

TcpClient::~TcpClient()
{
	m_running = false;
	if (m_thread.joinable())
		m_thread.join();

	TcpClose_();
	WSACleanup();
}

int TcpClient::TcpCreate()
{
	TcpClose_();

	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == m_socket)
	{
		return -1;
	}

	return 0;
}

int TcpClient::TcpConnectByTime(const char* ip, const int port, int seconds)
{
	if (INVALID_SOCKET == m_socket || seconds <= 0)
		return -1;

	if (0 != TcpSetNoBlock_(true) || 0 != TcpTimeout_(seconds))
		return -1;
	
	m_sockAddr.sin_family = AF_INET;
	m_sockAddr.sin_port = htons(port);
	m_sockAddr.sin_addr.S_un.S_addr = inet_addr(ip);
	return connect(m_socket, (const sockaddr*)&m_sockAddr, sizeof(m_sockAddr));
}

int TcpClient::TcpSetNoBlock_(bool onoff)
{
	if (INVALID_SOCKET == m_socket)
		return -1;

	DWORD val = (onoff ? 1 : 0);
	return ioctlsocket(m_socket, FIONBIO, &val);
}

int TcpClient::TcpTimeout_(int seconds)
{
	if (INVALID_SOCKET == m_socket || seconds <= 0)
		return -1;

	struct timeval tv;
	tv.tv_sec  = seconds; //Ãë
	tv.tv_usec = 0;       //Î¢Ãë
	return setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
}

int TcpClient::TcpRecv_(void* buf, int len)
{
	if (INVALID_SOCKET == m_socket)
		return -1;

	return recv(m_socket, (char*)buf, len, 0);
}

void TcpClient::TcpClose_()
{
	if (INVALID_SOCKET != m_socket)
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}
}

void TcpClient::TcpDataWorker()
{
	std::shared_ptr<char> recvBuf(new char[TCP_DATA_SIZE], std::default_delete<char[]>());
	memset(recvBuf.get(), 0x00, TCP_DATA_SIZE);

	while (m_running)
	{
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