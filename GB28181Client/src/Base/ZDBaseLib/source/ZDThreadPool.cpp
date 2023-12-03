#include "ZDThreadPool.h"

int RunInThread(void* param)
{
	assert(param);
	ZDThreadPool* pool = (ZDThreadPool*)param;
	pool->ThreadsWorker();
	return 0;
}

ZDThreadPool::ZDThreadPool(int tasks)
	: m_queue(tasks)
{
}

ZDThreadPool::~ZDThreadPool(void)
{
	Stop();
}

void ZDThreadPool::Start(int threads)
{
	m_running = true;

	for (int i = 0; i < threads; i++)
	{
		m_threadGroup.push_back(std::make_shared<std::thread>(RunInThread, this));
	}
}

void ZDThreadPool::Stop()
{
	//��֤���߳������ֻ����һ��StopThreadGroup_
	std::call_once(m_flag, [this] {
		StopThreadGroup_();
		});
}

void ZDThreadPool::AddTask(Task&& task)
{
	m_queue.Put(std::forward<Task>(task));
}

void ZDThreadPool::AddTask(const Task& task)
{
	m_queue.Put(task);
}

void ZDThreadPool::StopThreadGroup_()
{
	m_queue.Stop();        //ͬ�������߳�ֹͣ
	m_running = false;     //�ڲ��߳�����ѭ�����Ƴ�

	for (auto t : m_threadGroup)
	{
		if (t->joinable())
			t->join();
	}

	m_threadGroup.clear();
}

void ZDThreadPool::ThreadsWorker()
{
	while (m_running)
	{
		//ȡ����ֱ�ִ��
		std::list<Task> list;
		m_queue.Take(list);

		for (auto& task : list)
		{
			if (!m_running)
				return;

			task();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
}