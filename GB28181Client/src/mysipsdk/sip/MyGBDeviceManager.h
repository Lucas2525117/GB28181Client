#ifndef _MY_GB_DEVICE_GROUP_H_
#define _MY_GB_DEVICE_GROUP_H_

#include "MyGBDevice.h"
#include <unordered_map>
#include <mutex>

class CMyGBDeviceManager
{
public:
	static CMyGBDeviceManager& GetInstance()
	{
		static CMyGBDeviceManager gbDeviceManager;
		return gbDeviceManager;
	}

	// �����豸���
	MyGBDevicePtr Add(const std::string& user, const std::string& addr, int port);

	// �����豸ɾ��
	void Remove(const std::string& user);

	// ��ȡȫ���豸
	std::unordered_map<std::string, MyGBDevicePtr>& GetAllDevice();

	MyGBDevicePtr GetDeviceById(const std::string& gbid);

private:
	CMyGBDeviceManager();
	~CMyGBDeviceManager();

private:
	typedef std::unordered_map<std::string, MyGBDevicePtr> GBDeviceGroupMap;
	GBDeviceGroupMap m_devices;

	typedef std::lock_guard<std::recursive_mutex> RecursiveGuard;
	std::recursive_mutex m_recursive_mutex;   //�ݹ���
};

#endif //_MY_GB_DEVICE_GROUP_H_

