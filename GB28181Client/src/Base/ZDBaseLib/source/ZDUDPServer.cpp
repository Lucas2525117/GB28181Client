#include "ZDUDPServer.h"

static int UdpServerDataThread(void* param)
{
	assert(param);
	ZDUDPServer* client = (ZDUDPServer*)param;
	client->UdpDataWorker();
	return 0;
}

ZDUDPServer::ZDUDPServer(bool isUseCallback, UdpServerDataCallBack func, void* userdata)
	: m_isUseCallback(isUseCallback)
	, m_func(func)
	, m_userdata(userdata)
	, m_udpSocket(INVALID_SOCKET)
	, m_localAddr({ 0 })
	, m_remoteAddr({ 0 })
{
	// ³õÊ¼»¯Ì×½Ó×Ö
	if (0 == WSAStartup(MAKEWORD(2, 2), &m_wsaData))
	{
		if (m_isUseCallback)
		{
			m_running = true;
			m_thread = std::thread(UdpServerDataThread, this);
		}
	}
}

ZDUDPServer::~ZDUDPServer()
{
	UdpDestroy();
	WSACleanup();
}

int ZDUDPServer::UdpCreate()
{
	UdpClose_();

	m_udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == m_udpSocket)
	{
		return -1;
	}

	return 0;
}

void ZDUDPServer::UdpDestroy()
{
	UdpClose_();
}

int ZDUDPServer::UdpBind(unsigned short port)
{
	if (INVALID_SOCKET == m_udpSocket || port <= 0)
		return -1;

	m_localAddr.sin_family = AF_INET;
	m_localAddr.sin_port = htons(port);
	// localAddr.sin_addr.S_un.S_addr = INADDR_ANY; // 0.0.0.0
	m_localAddr.sin_addr.S_un.S_addr = inet_addr("0.0.0.0");
	return bind(m_udpSocket, (const sockaddr*)&m_localAddr, sizeof(m_localAddr));
}

int ZDUDPServer::UdpRecvfrom(void* data, int len)
{
	if (!data || len <= 0)
		return -1;

	return UdpRecvfrom_(data, len);
}

bool ZDUDPServer::UdpSend(const char* data, int len)
{
	if (INVALID_SOCKET == m_udpSocket || !data || len <= 0)
		return false;

	int ret = sendto(m_udpSocket, data, len, 0, (sockaddr*)&m_remoteAddr, sizeof(m_remoteAddr));
	if (SOCKET_ERROR == ret)
	{
		return false;
	}

	return true;
}

int ZDUDPServer::UdpRecvfrom_(void* data, int len)
{
	if (INVALID_SOCKET == m_udpSocket)
		return -1;

	int lenRemoteAddr = sizeof(m_remoteAddr);
	return recvfrom(m_udpSocket, (char*)data, len, 0, (SOCKADDR*)&m_remoteAddr, &lenRemoteAddr);
}

void ZDUDPServer::UdpClose_()
{
	if (INVALID_SOCKET != m_udpSocket)
	{
		closesocket(m_udpSocket);
		m_udpSocket = INVALID_SOCKET;
	}
}

void ZDUDPServer::UdpDataWorker()
{
	auto recvBuf = std::shared_ptr<char>(new char[UDP_SERVER_DATA_SIZE], std::default_delete<char[]>());
	memset(recvBuf.get(), 0, UDP_SERVER_DATA_SIZE);
	int recvLen = 0;

	while (m_running)
	{
		recvLen = UdpRecvfrom_(recvBuf.get(), UDP_SERVER_DATA_SIZE);
		if (recvLen <= 0)
			continue;

		if (m_func)
			m_func(recvBuf.get(), recvLen, m_userdata);
	}
}
