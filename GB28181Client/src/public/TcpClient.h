#ifndef _GB_TCP_CLIENT_H_
#define _GB_TCP_CLIENT_H_

#include <thread>
#include <assert.h>
#include <winSock2.h>

#define SOCKET_TIMEDOUT -WSAETIMEDOUT
#define TCP_DATA_SIZE (65536)

typedef void(*TcpDataCallBack)(void* data, int len, void* userData);

class TcpClient
{
public:
	TcpClient(TcpDataCallBack func, void* userdata);
	~TcpClient();

	// ����ֵ: 0-�ɹ�����0-ʧ�ܡ�
	int TcpCreate();

	// ip: ����˵�ip��ַ��
	// port: ����˼����Ķ˿ڡ�
	// ����ֵ: 0-�ɹ�����0-ʧ�ܡ�
	int TcpConnectByTime(const char* ip, const int port, int seconds);

	int TcpSetNoBlock(bool onoff);

	int TcpRecvTimeout(int seconds);

	int TcpSend(void* buf, int len, int flags, int timeout);

	int TcpRecv(void* buf, int len);

	void TcpDestroy();

	SOCKET GetClientSocket() const { return m_socket; }

	enum RECV_STATUS
	{
		RECV_HEAD = 1,   // ��������ͷ
		RECV_BODY = 2,   // ������������
	};

public:
	void TcpDataWorker();

private:
	int TcpSetNoBlock_(bool onoff);
	int TcpRecvTimeout_(int seconds);
	int TcpSend_(const char* buf, int len, int flags, int timeout);
	int TcpRecv_(void* buf, int len);
	int TcpSelectWrite_(int timeout);
	int TcpSelectWriteFds_(int n, fd_set* fds, struct timeval* timeout);
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
