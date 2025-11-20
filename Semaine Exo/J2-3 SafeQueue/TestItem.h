#ifndef TESTITEM_H
#define TESTITEM_H

struct TestItem {
    int producerId;
    int value;
    DWORD timestamp;

    TestItem() : producerId(0), value(0), timestamp(0) {}

    TestItem(int pId, int val)
        : producerId(pId), value(val), timestamp(GetTickCount()) {}
};

#endif // TESTITEM_H
