#ifndef _ZD_TIMER_H_
#define _ZD_TIMER_H_

#include <chrono>
#include <functional>
#include <list>
#include <mutex>
#include <thread>
#include <vector>

class ZDTimer
{
public:
	using Task = std::function<void()>;
    explicit ZDTimer(int tasks, int intervalMs);
	~ZDTimer();

	void Start();

	void Stop();

	void AddTask(int timeoutMs, Task task);

private:
	void GetTick_();

private:
    typedef std::vector<std::list<Task>> TasksVec;
    TasksVec m_vecTasks;
    size_t m_currentIndex;
    int m_tasksSize;
    int m_intervalMs;
    
    std::thread m_thread;
    bool m_running = false;
    std::mutex m_mutex;

};

#endif

