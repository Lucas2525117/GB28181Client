#ifndef _ZD_UDP_SERVER_H_
#define _ZD_UDP_SERVER_H_

#include <string>
#include <thread>
#include <memory>
#include <assert.h>
#include <Winsock2.h>

#define UDP_SERVER_DATA_SIZE 65535

typedef void(*UdpServerDataCallBack)(void* data, int len, void* userData);

class ZDUDPServer
{
public:
    ZDUDPServer(bool isUseCallback = false, UdpServerDataCallBack func = nullptr, void* userdata = nullptr);
    ~ZDUDPServer();

    int UdpCreate();

    void UdpDestroy();

    int UdpBind(unsigned short port);

    int UdpRecvfrom(void* data, int len);

    bool UdpSend(const char* data, int len);

public:
    void UdpDataWorker();

private:
    int UdpRecvfrom_(void* data, int len);
    void UdpClose_();

private:
    bool  m_isUseCallback = false;
    UdpServerDataCallBack m_func;
    void* m_userdata;

    WSADATA     m_wsaData;
    SOCKET      m_udpSocket;
    sockaddr_in m_localAddr;
    sockaddr_in m_remoteAddr; 

    std::thread m_thread;
    bool m_running;
};

typedef std::shared_ptr<ZDUDPServer> ZDUDPServerPtr;

#endif

