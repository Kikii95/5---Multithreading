#pragma once
#include <queue>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

template<typename T>
class SafeQueue {
private:
    std::queue<T> mQueue;
    CRITICAL_SECTION mCS;

public:
    SafeQueue() {
        InitializeCriticalSection(&mCS);
    }

    ~SafeQueue() {
        DeleteCriticalSection(&mCS);
    }

    SafeQueue(const SafeQueue&) = delete;
    SafeQueue& operator=(const SafeQueue&) = delete;

    void Push(T item) {
        EnterCriticalSection(&mCS);
        mQueue.push(item);
        LeaveCriticalSection(&mCS);
    }

    bool Pop(T& item) {
        EnterCriticalSection(&mCS);
        if (mQueue.empty()) {
            LeaveCriticalSection(&mCS);
            return false;
        }
        item = mQueue.front();
        mQueue.pop();
        LeaveCriticalSection(&mCS);
        return true;
    }

    bool IsEmpty() {
        EnterCriticalSection(&mCS);
        bool empty = mQueue.empty();
        LeaveCriticalSection(&mCS);
        return empty;
    }

    size_t Size() {
        EnterCriticalSection(&mCS);
        size_t size = mQueue.size();
        LeaveCriticalSection(&mCS);
        return size;
    }
};
