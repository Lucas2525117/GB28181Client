// �̳߳�
#ifndef _ZD_THREADPOOL_H_
#define _ZD_THREADPOOL_H_

#include <functional>
#include <thread>
#include <atomic>
#include <assert.h>
#include "ZDSyncQueue.h"

class ZDThreadPool
{
public:
	using Task = std::function<void()>;
	ZDThreadPool(int tasks = std::thread::hardware_concurrency());
	~ZDThreadPool(void); 

	void Start(int threads);

	void Stop();

	void AddTask(Task&& task);

	void AddTask(const Task& task);

public:
	void ThreadsWorker();

private:
	void StopThreadGroup_();

private:
	typedef std::list<std::shared_ptr<std::thread>> ThreadGroup; 
	ThreadGroup	             m_threadGroup;       //����������߳���
	ZDSyncQueue<Task>        m_queue;             //ͬ������
	std::atomic_bool         m_running;           //ֹͣ��־
	std::once_flag           m_flag;
};

#endif

