#ifndef SAFEQUEUE_H
#define SAFEQUEUE_H

#include <windows.h>

// Thread-safe queue using CRITICAL_SECTION and CONDITION_VARIABLE
template<typename T, int SIZE>
class SafeQueue {
private:
    T buffer[SIZE];
    int head;
    int tail;
    int count;

    CRITICAL_SECTION cs;
    CONDITION_VARIABLE cvNotEmpty;
    CONDITION_VARIABLE cvNotFull;

public:
    SafeQueue() : head(0), tail(0), count(0) {
        InitializeCriticalSection(&cs);
        InitializeConditionVariable(&cvNotEmpty);
        InitializeConditionVariable(&cvNotFull);
    }

    ~SafeQueue() {
        DeleteCriticalSection(&cs);
    }

    // Push item to queue (blocks if full)
    void push(const T& item) {
        EnterCriticalSection(&cs);

        // Wait while queue is full
        while (count == SIZE) {
            SleepConditionVariableCS(&cvNotFull, &cs, INFINITE);
        }

        // Add item to queue
        buffer[tail] = item;
        tail = (tail + 1) % SIZE;
        count++;

        // Wake up one consumer
        WakeConditionVariable(&cvNotEmpty);

        LeaveCriticalSection(&cs);
    }

    // Try push (non-blocking, returns false if full)
    bool tryPush(const T& item) {
        EnterCriticalSection(&cs);

        if (count == SIZE) {
            LeaveCriticalSection(&cs);
            return false;
        }

        buffer[tail] = item;
        tail = (tail + 1) % SIZE;
        count++;

        WakeConditionVariable(&cvNotEmpty);

        LeaveCriticalSection(&cs);
        return true;
    }

    // Pop item from queue (blocks if empty)
    T pop() {
        EnterCriticalSection(&cs);

        // Wait while queue is empty
        while (count == 0) {
            SleepConditionVariableCS(&cvNotEmpty, &cs, INFINITE);
        }

        // Remove item from queue
        T item = buffer[head];
        head = (head + 1) % SIZE;
        count--;

        // Wake up one producer
        WakeConditionVariable(&cvNotFull);

        LeaveCriticalSection(&cs);
        return item;
    }

    // Try pop (non-blocking, returns false if empty)
    bool tryPop(T& item) {
        EnterCriticalSection(&cs);

        if (count == 0) {
            LeaveCriticalSection(&cs);
            return false;
        }

        item = buffer[head];
        head = (head + 1) % SIZE;
        count--;

        WakeConditionVariable(&cvNotFull);

        LeaveCriticalSection(&cs);
        return true;
    }

    // Get current size
    int size() {
        EnterCriticalSection(&cs);
        int s = count;
        LeaveCriticalSection(&cs);
        return s;
    }

    // Check if empty
    bool isEmpty() {
        return size() == 0;
    }

    // Check if full
    bool isFull() {
        return size() == SIZE;
    }
};

#endif // SAFEQUEUE_H
