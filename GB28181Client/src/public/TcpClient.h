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

	// ����ֵ: 0-�ɹ�����0-ʧ�ܡ�
	int TcpCreate();

	// ip: ����˵�ip��ַ��
	// port: ����˼����Ķ˿ڡ�
	// ����ֵ: 0-�ɹ�����0-ʧ�ܡ�
	int TcpConnectByTime(const char* ip, const int port, int seconds);

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
