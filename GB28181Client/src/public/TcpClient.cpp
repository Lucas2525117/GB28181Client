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
		//m_thread = std::thread(TcpDataThread, this);
	}
}

TcpClient::~TcpClient()
{
	TcpDestroy();
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
	
	m_sockAddr.sin_family = AF_INET;
	m_sockAddr.sin_port = htons(port);
	m_sockAddr.sin_addr.S_un.S_addr = inet_addr(ip);
	return connect(m_socket, (const sockaddr*)&m_sockAddr, sizeof(m_sockAddr));
}

void TcpClient::TcpDestroy()
{
	m_running = false;
	if (m_thread.joinable())
		m_thread.join();

	TcpClose_();
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
	int recvLen = 0;

	while (m_running)
	{
		if (RECV_HEAD == m_status)
		{
			recvLen = 2;
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

		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}