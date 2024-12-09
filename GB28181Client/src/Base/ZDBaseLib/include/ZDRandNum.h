#ifndef _ZD_RAND_NUM_H_
#define _ZD_RAND_NUM_H_

#if defined(_MSC_VER) && _MSC_VER>1000
#pragma once
#endif

#if defined(OS_WINDOWS)
#include <Windows.h>
#endif

#include <random>

// 32 λ�������������
class ZDRandomGenerator32 {
private:
	std::mt19937 generator; // 32-bit Mersenne Twister
public:
	// ���캯����ʹ��ʱ����ΪĬ������
	ZDRandomGenerator32() {
		std::random_device rd;
		generator.seed(rd());
	}

	// ���캯����������̶�����
	explicit ZDRandomGenerator32(unsigned int seed) {
		generator.seed(seed);
	}

	// ����һ����������Χ [min, max]
	int generateInt(int min, int max) {
		std::uniform_int_distribution<int> dist(min, max);
		return dist(generator);
	}

	// ����һ������������Χ [min, max]
	double generateDouble(double min, double max) {
		std::uniform_real_distribution<double> dist(min, max);
		return dist(generator);
	}
};

// 64 λ�������������
class ZDRandomGenerator64 {
private:
	std::mt19937_64 generator; // 64-bit Mersenne Twister
public:
	// ���캯����ʹ��ʱ����ΪĬ������
	ZDRandomGenerator64() {
		std::random_device rd;
		generator.seed(rd());
	}

	// ���캯����������̶�����
	explicit ZDRandomGenerator64(unsigned long long seed) {
		generator.seed(seed);
	}

	// ����һ����������Χ [min, max]
	long long generateInt(long long min, long long max) {
		std::uniform_int_distribution<long long> dist(min, max);
		return dist(generator);
	}

	// ����һ������������Χ [min, max]
	double generateDouble(double min, double max) {
		std::uniform_real_distribution<double> dist(min, max);
		return dist(generator);
	}
};

#endif
