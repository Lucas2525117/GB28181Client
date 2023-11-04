#ifndef _GB_TCP_SERVER_H_
#define _GB_TCP_SERVER_H_

#include <thread>
#include <assert.h>
#include <winSock2.h>

#define TCP_DATA_SIZE (4*1024)

typedef void(*TcpDataCallBack)(void* data, int len, void* userData);

class TcpServer
{
public:
	TcpServer(TcpDataCallBack func, void* userdata);
	~TcpServer();

	int TcpCreate();

	// port: 绑定的端口
	int TcpBind(int port);

	// num: tcp三次握手后、accept之前的队列数
	int TcpListen(int num);

	void TcpDestroy();

public:
	void TcpDataWorker();

private:
	int TcpSetNoBlock_(bool onoff);
	int TcpTimeout_(int seconds);
	int TcpAccept_();
	int TcpRecv_(void* buf, int len);
	void TcpClose_();

public:
	TcpDataCallBack m_func;
	void* m_userdata;

	SOCKET  m_socketS; 
	sockaddr_in m_sockAddrS;

	SOCKET m_socketC;
	sockaddr_in m_sockAddrC;

	std::thread m_thread;
	bool m_running;
};

typedef std::shared_ptr<TcpServer> TcpServerPtr;

#endif  // _GB_TCP_SERVER_H_
