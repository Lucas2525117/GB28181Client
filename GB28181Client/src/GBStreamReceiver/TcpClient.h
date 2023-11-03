#ifndef _GB_TCP_CLIENT_H_
#define _GB_TCP_CLIENT_H_

#include <thread>
#include <assert.h>
#include <winSock2.h>

#define TCP_DATA_SIZE (4*1024)

typedef void(*TcpDataCallBack)(void* data, int len, void* userData);

class TcpClient
{
public:
	TcpClient(TcpDataCallBack func, void* userdata);
	~TcpClient();

	// 返回值: 0-成功；非0-失败。
	int TcpCreate();

	// ip: 服务端的ip地址。
	// port: 服务端监听的端口。
	// 返回值: 0-成功；非0-失败。
	int TcpConnectByTime(const char* ip, const int port, int seconds);

public:
	void TcpDataWorker();

private:
	int TcpSetNoBlock_(bool onoff);
	int TcpTimeout_(int seconds);
	int TcpRecv_(void* buf, int len);
	void TcpClose_();

public:
	TcpDataCallBack m_func;
	void* m_userdata;

	SOCKET  m_socket; 
	sockaddr_in m_sockAddr;
	std::thread m_thread;
	bool m_running;
};

typedef std::shared_ptr<TcpClient> TcpClientPtr;

#endif
