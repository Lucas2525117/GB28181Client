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
	std::atomic<bool> interrupt_flag;  // �����жϵı�־
	std::thread sleep_thread;          // ִ��˯�ߵ��߳�

	// ˽�г�Ա������ִ��ʵ�ʵ� sleep ����
	void sleep_task(int seconds) {
		using namespace std::chrono;
		auto start = steady_clock::now();
		auto end = start + std::chrono::seconds(std::chrono::seconds(seconds));

		// ��ָ��ʱ���ڼ���жϱ�־
		while (steady_clock::now() < end) {
			if (interrupt_flag.load()) {
				return;
			}

			std::this_thread::sleep_for(milliseconds(100));  // ÿ 100 ������һ��
		}
	}

public:
	// ���캯��
	ZDSInterruptibleSleep() : interrupt_flag(false) {}

	// �����ɴ�ϵ� sleep ����
	void start_sleep(int seconds) {
		// ����Ѿ���һ��˯���߳������У��ȵȴ�������
		if (sleep_thread.joinable()) {
			sleep_thread.join();
		}

		// ����һ�����߳�ִ�� sleep
		interrupt_flag.store(false);  // ȷ��û���ж�
		sleep_thread = std::thread(&ZDSInterruptibleSleep::sleep_task, this, seconds);
	}

	// �ж�˯��
	void interrupt() {
		interrupt_flag.store(true);  // ���ñ�־�ж�˯��
	}

	// �ȴ��߳����
	void join() {
		if (sleep_thread.joinable()) {
			sleep_thread.join();
		}
	}
};

#endif