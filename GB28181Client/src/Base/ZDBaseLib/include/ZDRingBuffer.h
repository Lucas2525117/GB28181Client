#ifndef _ZD_RING_BUFFER_H_
#define _ZD_RING_BUFFER_H_

#if defined(_MSC_VER) && _MSC_VER>1000
#pragma once
#endif

#if defined(OS_WINDOWS)
#include <Windows.h>
#endif

#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <optional>

template <typename T>
class ZDThreadSafeCircularBuffer {
private:
    std::vector<T> buffer;            // 环形缓冲区
    int head;                         // 读指针
    int tail;                         // 写指针
    int capacity;                     // 缓冲区容量
    int size;                         // 当前缓冲区中元素的数量

    std::mutex mtx;                   // 互斥锁
    std::condition_variable notEmpty; // 缓冲区非空条件变量
    std::condition_variable notFull;  // 缓冲区非满条件变量

public:
    // 构造函数
    explicit ZDThreadSafeCircularBuffer(int capacity)
        : buffer(capacity), head(0), tail(0), capacity(capacity), size(0) {}

    // 写入数据
    void write(const T& value) {
        std::unique_lock<std::mutex> lock(mtx);
        notFull.wait(lock, [this]() { return size < capacity; }); // 等待缓冲区非满

        buffer[tail] = value;
        tail = (tail + 1) % capacity;
        ++size;

        notEmpty.notify_one(); // 通知有新数据可读
    }

    // 读取数据
    T read() {
        std::unique_lock<std::mutex> lock(mtx);
        notEmpty.wait(lock, [this]() { return size > 0; }); // 等待缓冲区非空

        T value = buffer[head];
        head = (head + 1) % capacity;
        --size;

        notFull.notify_one(); // 通知有新空间可写
        return value;
    }

    // 非阻塞写入
    bool tryWrite(const T& value) {
        std::lock_guard<std::mutex> lock(mtx);
        if (size >= capacity) {
            return false; // 缓冲区已满，写入失败
        }

        buffer[tail] = value;
        tail = (tail + 1) % capacity;
        ++size;

        notEmpty.notify_one(); // 通知有新数据可读
        return true;
    }

    // 非阻塞读取
    std::optional<T> tryRead() {
        std::lock_guard<std::mutex> lock(mtx);
        if (size == 0) {
            return std::nullopt; // 缓冲区为空，读取失败
        }

        T value = buffer[head];
        head = (head + 1) % capacity;
        --size;

        notFull.notify_one(); // 通知有新空间可写
        return value;
    }

    // 检查缓冲区是否为空
    bool isEmpty() const {
        std::lock_guard<std::mutex> lock(mtx);
        return size == 0;
    }

    // 检查缓冲区是否已满
    bool isFull() const {
        std::lock_guard<std::mutex> lock(mtx);
        return size == capacity;
    }

    // 获取缓冲区当前大小
    int getSize() const {
        std::lock_guard<std::mutex> lock(mtx);
        return size;
    }

    // 获取缓冲区容量
    int getCapacity() const {
        return capacity;
    }
};

#endif
