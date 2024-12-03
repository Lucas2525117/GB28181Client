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
    std::vector<T> buffer;            // ���λ�����
    int head;                         // ��ָ��
    int tail;                         // дָ��
    int capacity;                     // ����������
    int size;                         // ��ǰ��������Ԫ�ص�����

    std::mutex mtx;                   // ������
    std::condition_variable notEmpty; // �������ǿ���������
    std::condition_variable notFull;  // ������������������

public:
    // ���캯��
    explicit ZDThreadSafeCircularBuffer(int capacity)
        : buffer(capacity), head(0), tail(0), capacity(capacity), size(0) {}

    // д������
    void write(const T& value) {
        std::unique_lock<std::mutex> lock(mtx);
        notFull.wait(lock, [this]() { return size < capacity; }); // �ȴ�����������

        buffer[tail] = value;
        tail = (tail + 1) % capacity;
        ++size;

        notEmpty.notify_one(); // ֪ͨ�������ݿɶ�
    }

    // ��ȡ����
    T read() {
        std::unique_lock<std::mutex> lock(mtx);
        notEmpty.wait(lock, [this]() { return size > 0; }); // �ȴ��������ǿ�

        T value = buffer[head];
        head = (head + 1) % capacity;
        --size;

        notFull.notify_one(); // ֪ͨ���¿ռ��д
        return value;
    }

    // ������д��
    bool tryWrite(const T& value) {
        std::lock_guard<std::mutex> lock(mtx);
        if (size >= capacity) {
            return false; // ������������д��ʧ��
        }

        buffer[tail] = value;
        tail = (tail + 1) % capacity;
        ++size;

        notEmpty.notify_one(); // ֪ͨ�������ݿɶ�
        return true;
    }

    // ��������ȡ
    std::optional<T> tryRead() {
        std::lock_guard<std::mutex> lock(mtx);
        if (size == 0) {
            return std::nullopt; // ������Ϊ�գ���ȡʧ��
        }

        T value = buffer[head];
        head = (head + 1) % capacity;
        --size;

        notFull.notify_one(); // ֪ͨ���¿ռ��д
        return value;
    }

    // ��黺�����Ƿ�Ϊ��
    bool isEmpty() const {
        std::lock_guard<std::mutex> lock(mtx);
        return size == 0;
    }

    // ��黺�����Ƿ�����
    bool isFull() const {
        std::lock_guard<std::mutex> lock(mtx);
        return size == capacity;
    }

    // ��ȡ��������ǰ��С
    int getSize() const {
        std::lock_guard<std::mutex> lock(mtx);
        return size;
    }

    // ��ȡ����������
    int getCapacity() const {
        return capacity;
    }
};

#endif
