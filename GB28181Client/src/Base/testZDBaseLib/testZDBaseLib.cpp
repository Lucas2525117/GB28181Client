// testZDBaseLib.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>
#include "ZDFile.h"
#include "ZDLRUCache.h"
#include "ZDSyncQueue.h"
#include "ZDThreadPool.h"
#include "ZDTime.h"
#include "ZDMemPool.h"
#include "ZDSafeDeque.h"
#include "ZDInterruptibleSleep.h"

using namespace std;

#define _IS_TEST_FILE_            0
#define _IS_TEST_LRU_             0
#define _IS_TEST_SYNCQUEUE_       0
#define _IS_TEST_THREADPOOL_      0
#define _IS_TEST_TIME_            0
#define _IS_TEST_MEMPOOL_         0
#define _IS_TEST_SAFE_DEQUE_      0
#define _IS_TEST_INTERRUPT_SLEEP_ 1

#if _IS_TEST_MEMPOOL_
class ActualClass
{
private:
    int count = 0;
    int No = 0;

public:
    ActualClass()
    {
        No = count;
        count++;
    }

    void print()
    {
        cout << this << ": ";
        cout << "the " << No << "th object" << endl;
    }

    void* operator new(size_t size);
    void operator delete(void* p);
};

//定义内存池对象
ZDMemPool<sizeof(ActualClass), 2> mp;

void* ActualClass::operator new(size_t size)
{
    return mp.ZD_Malloc();
}

void ActualClass::operator delete(void* p)
{
    mp.ZD_Free(p);
}
#endif

int main()
{
#if _IS_TEST_INTERRUPT_SLEEP_
    ZDSInterruptibleSleep sleep_obj;

	cout << "Starting interruptible sleep..." << endl;

	// 启动一个可打断的 sleep，设定睡眠时间为 5 秒
	sleep_obj.start_sleep(5);

	// 模拟一些操作，2 秒后中断睡眠
	this_thread::sleep_for(chrono::seconds(2));
	sleep_obj.interrupt();

	// 等待睡眠线程结束
	sleep_obj.join();
#elif _IS_TEST_SAFE_DEQUE_
	ZDSafeDeque<int> deque;

	deque.pushFront(10);
	deque.pushBack(20);
	deque.pushFront(5);

	std::cout << "Front: " << deque.front() << std::endl;
	std::cout << "Back: " << deque.back() << std::endl;

	deque.popFront();
	std::cout << "After popFront, Front: " << deque.front() << std::endl;

	deque.popBack();
	std::cout << "After popBack, Back: " << deque.back() << std::endl;
#elif _IS_TEST_FILE_
    if (!IsExistFile("C:\\rtp.tcp"))
    {
        printf("file is not existed\n");
    }

    if (!IsExistPath("C:\\dirTest"))
    {
        printf("dir is not existed\n");
    }

    printf("FileSize:%lld\n", GetFileSize("C:\\rtp.tcp"));

    if (!CreateDirPath("C:\\123TestDir"))
    {
        printf("Create path failed\n");
    }

#elif _IS_TEST_LRU_
    ZDLRUCahce cache(2);

    cache.Put(1, 1);
    cache.Put(2, 2);

    cache.Get(1);
    cache.Put(3, 3);

    cout << cache.Get(2) << endl;

    cache.Put(4, 4);

    cout << cache.Get(1) << endl;
    cout << cache.Get(3) << endl;
    cout << cache.Get(4) << endl;

    cache.Put(5, 5);

    cout << cache.Get(3) << endl;
    cout << cache.Get(4) << endl;
    cout << cache.Get(5) << endl;
#elif _IS_TEST_SYNCQUEUE_
    ZDSyncQueue<int> syncQueue(3);

    cout << "IsEmpty(): " << syncQueue.IsEmpty() << endl;

    syncQueue.Put(1);
    syncQueue.Put(2);
    syncQueue.Put(3);

    cout << "IsFull(): " << syncQueue.IsFull() << endl;

    int num = 0;
    syncQueue.Take(num);
    cout << num << endl;

    cout << "Size:" << syncQueue.Size() << endl;
    cout << "IsEmpty(): " << syncQueue.IsEmpty() << endl;

    std::list<int> lst; 
    syncQueue.Take(lst);
    for (auto it : lst)
    {
        cout << it << endl;
    }

    cout << "Size:" << syncQueue.Size() << endl;
    cout << "IsEmpty(): " << syncQueue.IsEmpty() << endl;

#elif _IS_TEST_THREADPOOL_
    ZDThreadPool pool;
    pool.Start(3);

    cout << "Main thread: " << std::this_thread::get_id() << endl;

    //std::function<void()> func = std::bind(TestThreadPoolFunc);
    auto func = std::bind([](int num) {
        for (int i = 0; i < num; i++)
        {
            cout << "Current thread: " << std::this_thread::get_id() << ":" << i << endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        }, 500);
    pool.AddTask(func);
#elif _IS_TEST_TIME_
    cout << "当前时间(秒):" << ZD_GetSystemTime_S() << endl;
    cout << "当前时间(毫秒):" << ZD_GetSystemTime_MS() << endl;
    cout << "当前时间(微秒):" << ZD_GetSystemTime_MicroS() << endl;
    cout << "当前时间(纳秒):" << ZD_GetSystemTime_NS() << endl;

    cout << "当前时间(字符串):" << ZD_FormatCurrentTime() << endl;
#elif _IS_TEST_MEMPOOL_
    /*ActualClass* p1 = new ActualClass;
    p1->print();

    ActualClass* p2 = new ActualClass;
    p2->print();
    delete p1;

    p1 = new ActualClass;
    p1->print();

    ActualClass* p3 = new ActualClass;
    p3->print();

    delete p1;
    delete p2;
    delete p3;*/

    ZDMemPool<sizeof(char), 2> p;
    char* p1 = (char*)p.ZD_Malloc();
    char* p2 = (char*)p.ZD_Malloc();
    char* p3 = (char*)p.ZD_Malloc();

    p.ZD_Free(p1);
    p.ZD_Free(p2);
    p.ZD_Free(p3);
    
#endif 

    system("pause");
    return 0;
}
