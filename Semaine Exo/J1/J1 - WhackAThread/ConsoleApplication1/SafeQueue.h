#ifndef SAFEQUEUE_H
#define SAFEQUEUE_H

#include <windows.h>

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

    void push(const T& item) {
        EnterCriticalSection(&cs);

        while (count == SIZE) {
            SleepConditionVariableCS(&cvNotFull, &cs, INFINITE);
        }

        buffer[tail] = item;
        tail = (tail + 1) % SIZE;
        count++;

        WakeConditionVariable(&cvNotEmpty);

        LeaveCriticalSection(&cs);
    }

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

    T pop() {
        EnterCriticalSection(&cs);

        while (count == 0) {
            SleepConditionVariableCS(&cvNotEmpty, &cs, INFINITE);
        }

        T item = buffer[head];
        head = (head + 1) % SIZE;
        count--;

        WakeConditionVariable(&cvNotFull);

        LeaveCriticalSection(&cs);
        return item;
    }

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

    int size() {
        EnterCriticalSection(&cs);
        int s = count;
        LeaveCriticalSection(&cs);
        return s;
    }

    bool isEmpty() {
        return size() == 0;
    }

    bool isFull() {
        return size() == SIZE;
    }
};

#endif // SAFEQUEUE_H
