// LRU(最近最少使用)缓存机制
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

	// 1.key不存在,返回-1
	// 2.key存在,list中删除key,在list头部重新插入,更新map
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

	// 1.key存在:删除该key,在list头部插入数据,更新map
	// 2.key不存在
	//// 2.1 容量未满:在list头部插入数据,更新map
	//// 2.2 容量已满:删除list尾部数据,在list头部重新插入数据,,更新map
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