#ifndef _ZD_RAND_NUM_H_
#define _ZD_RAND_NUM_H_

#if defined(_MSC_VER) && _MSC_VER>1000
#pragma once
#endif

#if defined(OS_WINDOWS)
#include <Windows.h>
#endif

#include <random>

// 32 位随机数生成器类
class ZDRandomGenerator32 {
private:
	std::mt19937 generator; // 32-bit Mersenne Twister
public:
	// 构造函数：使用时间作为默认种子
	ZDRandomGenerator32() {
		std::random_device rd;
		generator.seed(rd());
	}

	// 构造函数：允许传入固定种子
	explicit ZDRandomGenerator32(unsigned int seed) {
		generator.seed(seed);
	}

	// 生成一个整数，范围 [min, max]
	int generateInt(int min, int max) {
		std::uniform_int_distribution<int> dist(min, max);
		return dist(generator);
	}

	// 生成一个浮点数，范围 [min, max]
	double generateDouble(double min, double max) {
		std::uniform_real_distribution<double> dist(min, max);
		return dist(generator);
	}
};

// 64 位随机数生成器类
class ZDRandomGenerator64 {
private:
	std::mt19937_64 generator; // 64-bit Mersenne Twister
public:
	// 构造函数：使用时间作为默认种子
	ZDRandomGenerator64() {
		std::random_device rd;
		generator.seed(rd());
	}

	// 构造函数：允许传入固定种子
	explicit ZDRandomGenerator64(unsigned long long seed) {
		generator.seed(seed);
	}

	// 生成一个整数，范围 [min, max]
	long long generateInt(long long min, long long max) {
		std::uniform_int_distribution<long long> dist(min, max);
		return dist(generator);
	}

	// 生成一个浮点数，范围 [min, max]
	double generateDouble(double min, double max) {
		std::uniform_real_distribution<double> dist(min, max);
		return dist(generator);
	}
};

#endif
