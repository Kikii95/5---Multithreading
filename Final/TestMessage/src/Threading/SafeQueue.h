#pragma once
#include <queue>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

// File thread-safe (FIFO) avec Critical Section
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

    // Interdit copie/assignation
    SafeQueue(const SafeQueue&) = delete;
    SafeQueue& operator=(const SafeQueue&) = delete;

    // Push element (thread-safe)
    void Push(T item) {
        EnterCriticalSection(&mCS);
        mQueue.push(item);
        LeaveCriticalSection(&mCS);
    }

    // Pop element (thread-safe)
    // Retourne true si succes, false si queue vide
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

    // Verifier si vide
    bool IsEmpty() {
        EnterCriticalSection(&mCS);
        bool empty = mQueue.empty();
        LeaveCriticalSection(&mCS);
        return empty;
    }

    // Taille
    size_t Size() {
        EnterCriticalSection(&mCS);
        size_t size = mQueue.size();
        LeaveCriticalSection(&mCS);
        return size;
    }
};
