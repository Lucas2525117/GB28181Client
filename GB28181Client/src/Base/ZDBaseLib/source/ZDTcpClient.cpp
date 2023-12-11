#include "ZDTcpClient.h"

static int TcpDataThread(void* param)
{
	assert(param);
	ZDTcpClient* client = (ZDTcpClient*)param;
	client->TcpDataWorker();
	return 0;
}

ZDTcpClient::ZDTcpClient(TcpDataCallBack func, void* userdata)
	: m_func(func)
	, m_userdata(userdata)
	, m_socket(INVALID_SOCKET)
	, m_sockAddr({ 0 })
	, m_running(false)
{
	// 初始化套接字 
	WSADATA wsaData;
	if (0 == WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		m_running = true;
		//m_thread = std::thread(TcpDataThread, this);
	}
}

ZDTcpClient::~ZDTcpClient()
{
	TcpDestroy();
	WSACleanup();
}

int ZDTcpClient::TcpCreate()
{
	TcpClose_();

	// protocol为0时，会自动选择type类型对应的默认协议
	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == m_socket)
	{
		return -1;
	}

	return 0;
}

int ZDTcpClient::TcpCreate_ipv6()
{
	TcpClose_();

	m_socket = socket(PF_INET6, SOCK_STREAM, 0);
	if (INVALID_SOCKET == m_socket)
	{
		return -1;
	}

	return 0;
}

int ZDTcpClient::TcpConnect(const char* ip, const int port)
{
	if (INVALID_SOCKET == m_socket)
		return -1;
	
	m_sockAddr.sin_family = AF_INET;
	m_sockAddr.sin_port = htons(port);
	m_sockAddr.sin_addr.S_un.S_addr = inet_addr(ip);
	return connect(m_socket, (const sockaddr*)&m_sockAddr, sizeof(m_sockAddr));
}

int ZDTcpClient::TcpSetNoBlock(bool onoff)
{
	return TcpSetNoBlock_(onoff);
}

int ZDTcpClient::TcpRecvTimeout(int seconds)
{
	return TcpRecvTimeout_(seconds);
}

void ZDTcpClient::TcpDestroy()
{
	m_running = false;
	if (m_thread.joinable())
		m_thread.join();

	TcpClose_();
}

int ZDTcpClient::TcpSetNoBlock_(bool onoff)
{
	if (INVALID_SOCKET == m_socket)
		return -1;

	DWORD val = (onoff ? 1 : 0);
	return ioctlsocket(m_socket, FIONBIO, &val);
}

int ZDTcpClient::TcpRecvTimeout_(int seconds)
{
	if (INVALID_SOCKET == m_socket || seconds <= 0)
		return -1;

	struct timeval tv;
	tv.tv_sec = seconds;  //秒
	tv.tv_usec = 0;       //微秒
	return setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
}

int ZDTcpClient::TcpSend(void* buf, int len, int flags, int timeout)
{
	int r;
	size_t bytes = 0;

	while (bytes < len)
	{
		r = TcpSend_((const char*)buf + bytes, len - bytes, flags, timeout);
		if (r <= 0)
			return r;	// <0-error

		bytes += r;
	}

	return (int)bytes;
}

int ZDTcpClient::TcpRecv(void* buf, int len)
{
	return TcpRecv_(buf, len);
}

int ZDTcpClient::TcpSend_(const char* buf, int len, int flags, int timeout)
{
	if (INVALID_SOCKET == m_socket)
		return -1;

	/*int r = TcpSelectWrite_(timeout);
	if (r <= 0)
		return 0 == r ? SOCKET_TIMEDOUT : r;*/

	return send(m_socket, (char*)buf, len, 0);
}

int ZDTcpClient::TcpRecv_(void* buf, int len)
{
	if (INVALID_SOCKET == m_socket)
		return -1;

	return recv(m_socket, (char*)buf, len, 0);
}

int ZDTcpClient::TcpSelectWrite_(int timeout)
{
	if (INVALID_SOCKET == m_socket)
		return -1;

	fd_set fds;
	struct timeval tv;

	FD_ZERO(&fds);
	FD_SET(m_socket, &fds);

	tv.tv_sec = timeout / 1000;
	tv.tv_usec = (timeout % 1000) * 1000;
	return TcpSelectWriteFds_(0 /*sock+1*/, &fds, timeout < 0 ? NULL : &tv);
}

int ZDTcpClient::TcpSelectWriteFds_(int n, fd_set* fds, struct timeval* timeout)
{
	return select(n, fds, NULL, NULL, timeout);
}

void ZDTcpClient::TcpClose_()
{
	if (INVALID_SOCKET != m_socket)
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}
}

void ZDTcpClient::TcpDataWorker()
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