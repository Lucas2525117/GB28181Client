#ifndef _MY_GB_DEVICE_H_
#define _MY_GB_DEVICE_H_

#include <string>
#include <vector>
#include <memory>

#define DonameStartPos 0
#define DonameLength  10
#define TypeStartPos 10
#define TypeLength 3

class CMyGBDevice
{
public:
	CMyGBDevice() {} 
	~CMyGBDevice() {}

	void Init(const std::string& user, const std::string& addr, int port)
	{
		if (!m_hasInit)
		{
			m_user = user;
			m_sipCodecUrl = "SIP:" + user + "@" + user.substr(0, 10);
			m_sipIpUrl = "SIP:" + user + "@" + addr + ":" + std::to_string(port);
			m_deviceType = stoi(user.substr(TypeStartPos, TypeLength));
			m_deviceDomain = user.substr(DonameStartPos, DonameLength);
			m_addr = addr;
			m_port = port;
			m_hasInit = true;
		}
	}

	void Add(const std::string& deviceId)
	{
		m_deviceIds.push_back(deviceId);
	}

	std::string GetUser() const 
	{
		return m_user;
	}

	std::string GetSipCodecUrl() const 
	{
		return m_sipCodecUrl;
	}

	std::string GetSipIpUrl() const
	{
		return m_sipIpUrl;
	}

	std::string GetRemoteIpUrl(std::string remoteAddr, int remotePort)
	{
		return "SIP:" + m_user + "@" + remoteAddr + ":" + std::to_string(remotePort);
	}

	void SetNetAddr(std::string netIp, int netPort)
	{
		m_netIp = netIp;
		m_netPort = netPort;
	}

	std::string GetNetIP() const 
	{
		return m_netIp;
	}

	int GetNetPort() const 
	{
		return m_netPort;
	}

	std::string GetRemoteAddr() const
	{
		return m_addr;
	}

private:
	bool m_hasInit = false;
	std::string m_user;
	std::string m_deviceDomain;
	std::string m_addr;
	int m_port = 0;
	std::vector<std::string> m_deviceIds;
	int m_deviceType = 0;
	std::string m_sipCodecUrl;
	std::string m_sipIpUrl;
	std::string m_netIp;
	int m_netPort = 0;
};

typedef std::shared_ptr<CMyGBDevice> MyGBDevicePtr;

#endif