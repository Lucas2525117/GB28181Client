#ifndef _ZD_TCP_SERVER_H_
#define _ZD_TCP_SERVER_H_

#include <thread>
#include <assert.h>
#include <winSock2.h>

#define TCP_SERVER_DATA_SIZE 65536
#define RTP_HEAD_SIZE 2

typedef void(*TcpDataCallBack)(void* data, int len, void* userData);

class ZDTcpServer
{
public:
	ZDTcpServer(TcpDataCallBack func, void* userdata);
	~ZDTcpServer();

	int TcpCreate();
	int TcpCreate_ipv6();

	// port: 绑定的端口
	int TcpBind(int port);

	// num: tcp三次握手后、accept之前的队列数
	int TcpListen(int num);

	int TcpAccept();

	void TcpDestroy();

	SOCKET GetClientSocket() const;

	enum RECV_STATUS
	{
		RECV_HEAD = 1,   // 接收数据头
		RECV_BODY = 2,   // 接收数据内容
	};

public:
	void TcpDataWorker();

private:
	int TcpSetNoBlock_(bool onoff);
	int TcpTimeout_(int seconds);
	int TcpAccept_();
	int TcpRecv_(void* buf, int len);
	void TcpClose_();

	int BeginReceive_(void*& buf, int& len);
	int EndReceive_();

public:
	TcpDataCallBack m_func;
	void* m_userdata;

	SOCKET  m_socketS; 
	sockaddr_in m_sockAddrS;

	SOCKET m_socketC;
	sockaddr_in m_sockAddrC;

	std::thread m_thread;
	bool m_running;

	std::shared_ptr<char> m_recvBuf;
	int m_recvLen = 0;
	int m_status = RECV_HEAD;
};

typedef std::shared_ptr<ZDTcpServer> ZDTcpServerPtr;

#endif  // _GB_TCP_SERVER_H_
