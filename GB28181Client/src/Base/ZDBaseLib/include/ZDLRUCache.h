// LRU(�������ʹ��)�������
#ifndef _ZD_LRU_CACHE_H_
#define _ZD_LRU_CACHE_H_

#include <unordered_map>
#include <list>
#include <mutex>

class ZDLRUCahce
{
public:
	ZDLRUCahce(int capacity)
		: m_capacity(capacity)
	{
	}

	~ZDLRUCahce()
	{
	}

	// 1.key������,����-1
	// 2.key����,list��ɾ��key,��listͷ�����²���,����map
	int Get(int key)
	{
		RecursiveGuard mtx(m_recursive_mutex);
		auto it = m_mapCache.find(key);
		if (it == m_mapCache.end())
			return -1;

		std::pair<int, int> kv = *m_mapCache[key];
		m_listCache.erase(m_mapCache[key]);
		m_listCache.push_front(kv);
		m_mapCache[key] = m_listCache.begin();
		return kv.second;
	}

	// 1.key����:ɾ����key,��listͷ����������,����map
	// 2.key������
	//// 2.1 ����δ��:��listͷ����������,����map
	//// 2.2 ��������:ɾ��listβ������,��listͷ�����²�������,,����map
	void Put(int key, int value)
	{
		RecursiveGuard mtx(m_recursive_mutex);
		auto it = m_mapCache.find(key);
		if (it != m_mapCache.end())
		{
			m_listCache.erase(m_mapCache[key]);
			m_listCache.push_front(std::make_pair(key, value));
			m_mapCache[key] = m_listCache.begin();
			return;
		}

		if (m_listCache.size() == m_capacity)
		{
			auto iter = m_listCache.back();
			m_mapCache.erase(iter.first);
			m_listCache.pop_back();
			m_listCache.push_front(std::make_pair(key, value));
			m_mapCache[key] = m_listCache.begin();
		}
		else
		{
			m_listCache.push_front(std::make_pair(key, value));
			m_mapCache[key] = m_listCache.begin();
		}
	}

private:
	int m_capacity;

	typedef std::lock_guard<std::recursive_mutex> RecursiveGuard;
	typedef std::list<std::pair<int, int>> Cache;
	typedef std::unordered_map<int, Cache::iterator> CacheMap;
	std::recursive_mutex m_recursive_mutex;
	Cache      m_listCache;
	CacheMap   m_mapCache;
};

#endif