// ͬ������
#ifndef _ZD_SYNC_QUEUE_H_
#define _ZD_SYNC_QUEUE_H_

#include <list>
#include <mutex>
#include <condition_variable>

template <typename T>
class ZDSyncQueue
{
public:
	ZDSyncQueue(int maxSize)
		: m_maxSize(maxSize)
	{
	}

	void Put(const T& t)
	{
		Add_(t);
	}

	void Put(T&& t)
	{
		Add_(std::forward<T>(t));
	}

	void Take(std::list<T>& list)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_notEmpty.wait(lock, [this] {
			return m_stop || IsNotEmpty_();
			});                       // �����������㣬�߳���Ϊwaiting״̬

		if (m_stop)
			return;

		list = std::move(m_queue);    //��ȡͬ������
		m_notFull.notify_one();
	}

	void Take(T& t)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_notEmpty.wait(lock, [this] {
			return m_stop || IsNotEmpty_();
			});

		if (m_stop)
			return;

		t = m_queue.front();
		m_queue.pop_front();
		m_notFull.notify_one();
	}

	void Stop()
	{
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			m_stop = true;
		}

		m_notFull.notify_all();
		m_notEmpty.notify_all();
	}

	bool IsEmpty()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		return m_queue.empty();
	}

	bool IsFull()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		return m_queue.size() == m_maxSize;
	}

	size_t Size()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		return m_queue.size();
	}

	int Count()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		return m_queue.size();
	}

private:
	bool IsNotFull_() const
	{
		return !(m_queue.size() >= m_maxSize);
	}

	bool IsNotEmpty_() const
	{
		return !m_queue.empty();
	}

	template<typename F>
	void Add_(F&& f)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_notFull.wait(lock, [this] {
			return m_stop || IsNotFull_();
			});

		m_queue.push_back(std::forward<F>(f));     //����������������
		m_notEmpty.notify_one();                   //����ȡ�����߳�ȥȡ����
	}

private:
	typedef std::list<T> ListQueue;                     // ������

	std::mutex                m_mutex;
	ListQueue                 m_queue;
	std::condition_variable   m_notEmpty;               // ��Ϊ�յ���������(�̳߳��̲߳�Ϊ��)
	std::condition_variable   m_notFull;                // û��������������(�̳߳�����û����)

	int                       m_maxSize;
	bool                      m_stop = false;
};

#endif
