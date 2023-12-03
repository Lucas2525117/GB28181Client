#include "ZDTimer.h"

ZDTimer::ZDTimer(int tasks, int intervalMs)
    : m_tasksSize(tasks)
    , m_intervalMs(intervalMs)
    , m_vecTasks(tasks)
    , m_currentIndex(0)
{
}

ZDTimer::~ZDTimer()
{
    Stop();
}

void ZDTimer::Start()
{
    if (m_running) 
        return;
    
    m_running = true;
    m_thread = std::thread([this]() {
        while (m_running) 
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(m_intervalMs));
            GetTick_();
        }
        });
    m_thread.detach();
}

void ZDTimer::Stop()
{
    if (!m_running)
        return;
    
    m_running = false;
    if (m_thread.joinable()) 
    {
        m_thread.join();
    }

    m_vecTasks.clear();
}

void ZDTimer::AddTask(int timeoutMs, Task task)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_intervalMs <= 0 || m_tasksSize <= 0)
        return;

    size_t ticks = timeoutMs / m_intervalMs;
    size_t index = (m_currentIndex + ticks) % m_tasksSize;
    size_t allindex = index;
    for (size_t i = 1; allindex < m_tasksSize; i++)
    {
        allindex = index * i;
        if (allindex >= m_tasksSize)
            break;
        m_vecTasks[allindex].push_back(task);
    }
}

void ZDTimer::GetTick_()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (0 == m_vecTasks.size())
        return;

    auto& tasks = m_vecTasks[m_currentIndex];
    for (const auto& task : tasks) 
    {
        task();
    }
    //tasks.clear();
    m_currentIndex = (m_currentIndex + 1) % m_tasksSize;
}