#include "MyGBDeviceManager.h"

CMyGBDeviceManager::CMyGBDeviceManager()
{
}

CMyGBDeviceManager::~CMyGBDeviceManager()
{
}

MyGBDevicePtr CMyGBDeviceManager::Add(const std::string& user, const std::string& addr, int port)
{
	RecursiveGuard mtx(m_recursive_mutex);
	auto it = m_devices.find(user);
	if (it != m_devices.end())
		return nullptr;

	auto device = std::make_shared<CMyGBDevice>();
	device->Init(user, addr, port);
	m_devices.emplace(user, device);
	return device;
}

void CMyGBDeviceManager::Remove(const std::string& user)
{
	RecursiveGuard mtx(m_recursive_mutex);
	auto it = m_devices.find(user);
	if (it != m_devices.end())
		m_devices.erase(it);
}

std::unordered_map<std::string, MyGBDevicePtr>& CMyGBDeviceManager::GetAllDevice()
{
	RecursiveGuard mtx(m_recursive_mutex);
	return m_devices;
}

MyGBDevicePtr CMyGBDeviceManager::GetDeviceById(const std::string& gbid)
{
	RecursiveGuard mtx(m_recursive_mutex);
	for (const auto& it : m_devices)
	{
		if (it.first == gbid)
			return it.second;
	}

	return nullptr;
}
