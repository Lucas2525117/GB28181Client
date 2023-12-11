#ifndef _ZD_UDP_CLIENT_H_
#define _ZD_UDP_CLIENT_H_

#include <string>
#include <thread>
#include <assert.h>
#include <memory>
#include <Winsock2.h>

#define UDP_CLIENT_DATA_SIZE 65535

typedef void(*UdpClientDataCallBack)(void* data, int len, void* userData);

class ZDUDPClient
{
public:
    ZDUDPClient(bool isUseCallback = false, UdpClientDataCallBack func = nullptr, void* userdata = nullptr);
    ~ZDUDPClient();

    int UdpCreate();
    
    void UdpDestroy();

    int UdpBind(const std::string& ip, unsigned short port);

    int UdpRecvfrom(void* data, int len);

    bool UdpSend(const char* data, int len);

    int UdpSetNoblock(int noblock);  // 0-block, 1-no-block

public:
    void UdpDataWorker();

private:
    int UdpRecvfrom_(void* data, int len);
    void UdpClose_();

private:
    bool  m_isUseCallback = false;
    UdpClientDataCallBack m_func;
    void* m_userdata;

    WSADATA       m_wsaData;
    SOCKET        m_udpSocket;
    sockaddr_in   m_localAddr;
    sockaddr_in   m_remoteAddr;

    std::thread   m_thread;
    bool          m_running;
};

typedef std::shared_ptr<ZDUDPClient> ZDUDPClientPtr;

#endif

