#ifndef _ZD_SAFE_DEQUE_H_
#define _ZD_SAFE_DEQUE_H_

#if defined(_MSC_VER) && _MSC_VER>1000
#pragma once
#endif

#if defined(OS_WINDOWS)
#include <Windows.h>
#endif

#include <iostream>
#include <mutex>
#include <stdexcept>
#include <memory>

// 节点类
template <typename T>
class Node {
public:
	T data;
	std::shared_ptr<Node> prev;
	std::shared_ptr<Node> next;

	Node(T value) : data(value), prev(nullptr), next(nullptr) {}
};

// 线程安全的双向队列类
template <typename T>
class ZDSafeDeque {
private:
	std::shared_ptr<Node<T>> head;
	std::shared_ptr<Node<T>> tail;
	size_t size;
	mutable std::recursive_mutex mtx; // 保护队列操作的互斥锁

public:
	// 构造函数
	ZDSafeDeque() : head(nullptr), tail(nullptr), size(0) {}

	// 检查队列是否为空
	bool isEmpty() const {
		std::lock_guard<std::recursive_mutex> lock(mtx);
		return size == 0;
	}

	// 返回队列的大小
	size_t getSize() const {
		std::lock_guard<std::recursive_mutex> lock(mtx);
		return size;
	}

	// 从队列前端插入元素
	void pushFront(const T& value) {
		std::lock_guard<std::recursive_mutex> lock(mtx);
		auto newNode = std::make_shared<Node<T>>(value);
		if (isEmpty()) {
			head = tail = newNode;
		}
		else {
			newNode->next = head;
			head->prev = newNode;
			head = newNode;
		}
		size++;
	}

	// 从队列后端插入元素
	void pushBack(const T& value) {
		std::lock_guard<std::recursive_mutex> lock(mtx);
		auto newNode = std::make_shared<Node<T>>(value);
		if (isEmpty()) {
			head = tail = newNode;
		}
		else {
			newNode->prev = tail;
			tail->next = newNode;
			tail = newNode;
		}
		size++;
	}

	// 从队列前端删除元素
	void popFront() {
		std::lock_guard<std::recursive_mutex> lock(mtx);
		if (isEmpty()) {
			throw std::runtime_error("Deque is empty. Cannot pop front.");
		}
		auto temp = head;
		head = head->next;
		if (head) {
			head->prev.reset();
		}
		else {
			tail.reset();
		}
		size--;
	}

	// 从队列后端删除元素
	void popBack() {
		std::lock_guard<std::recursive_mutex> lock(mtx);
		if (isEmpty()) {
			throw std::runtime_error("Deque is empty. Cannot pop back.");
		}
		auto temp = tail;
		tail = tail->prev;
		if (tail) {
			tail->next.reset();
		}
		else {
			head.reset();
		}
		size--;
	}

	// 获取前端元素
	T front() const {
		std::lock_guard<std::recursive_mutex> lock(mtx);
		if (isEmpty()) {
			throw std::runtime_error("Deque is empty. No front element.");
		}
		return head->data;
	}

	// 获取后端元素
	T back() const {
		std::lock_guard<std::recursive_mutex> lock(mtx);
		if (isEmpty()) {
			throw std::runtime_error("Deque is empty. No back element.");
		}
		return tail->data;
	}
};


#endif
