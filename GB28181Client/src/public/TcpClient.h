#ifndef _GB_TCP_CLIENT_H_
#define _GB_TCP_CLIENT_H_

#include <thread>
#include <assert.h>
#include <winSock2.h>

#define TCP_DATA_SIZE (65536)

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

	void TcpDestroy();

	SOCKET GetClientSocket() const { return m_socket; }

	enum RECV_STATUS
	{
		RECV_HEAD = 1,   // 接收数据头
		RECV_BODY = 2,   // 接收数据内容
	};

public:
	void TcpDataWorker();

private:
	int TcpRecv_(void* buf, int len);
	void TcpClose_();

public:
	TcpDataCallBack m_func;
	void* m_userdata;

	SOCKET  m_socket; 
	sockaddr_in m_sockAddr;
	std::thread m_thread;
	bool m_running;
	int m_status = RECV_HEAD;
};

typedef std::shared_ptr<TcpClient> TcpClientPtr;

#endif
