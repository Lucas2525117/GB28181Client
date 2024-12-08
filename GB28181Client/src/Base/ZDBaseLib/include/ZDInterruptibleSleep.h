#ifndef _ZD_INTERRUPTIBLE_SLEEP_H_
#define _ZD_INTERRUPTIBLE_SLEEP_H_

#if defined(_MSC_VER) && _MSC_VER>1000
#pragma once
#endif

#if defined(OS_WINDOWS)
#include <Windows.h>
#endif

#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>

class ZDSInterruptibleSleep {
private:
	std::atomic<bool> interrupt_flag;  // 用于中断的标志
	std::thread sleep_thread;          // 执行睡眠的线程

	// 私有成员函数：执行实际的 sleep 操作
	void sleep_task(int seconds) {
		using namespace std::chrono;
		auto start = steady_clock::now();
		auto end = start + std::chrono::seconds(std::chrono::seconds(seconds));

		// 在指定时间内检查中断标志
		while (steady_clock::now() < end) {
			if (interrupt_flag.load()) {
				return;
			}

			std::this_thread::sleep_for(milliseconds(100));  // 每 100 毫秒检查一次
		}
	}

public:
	// 构造函数
	ZDSInterruptibleSleep() : interrupt_flag(false) {}

	// 启动可打断的 sleep 操作
	void start_sleep(int seconds) {
		// 如果已经有一个睡眠线程在运行，先等待它结束
		if (sleep_thread.joinable()) {
			sleep_thread.join();
		}

		// 启动一个新线程执行 sleep
		interrupt_flag.store(false);  // 确保没有中断
		sleep_thread = std::thread(&ZDSInterruptibleSleep::sleep_task, this, seconds);
	}

	// 中断睡眠
	void interrupt() {
		interrupt_flag.store(true);  // 设置标志中断睡眠
	}

	// 等待线程完成
	void join() {
		if (sleep_thread.joinable()) {
			sleep_thread.join();
		}
	}
};

#endif