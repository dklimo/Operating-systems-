#ifndef RECORD_LOCK_H
#define RECORD_LOCK_H

#include <windows.h>

struct RecordLock {
    CRITICAL_SECTION cs;
    int readers;
    bool writer;

    RecordLock() {
        InitializeCriticalSection(&cs);
        readers = 0;
        writer = false;
    }

    ~RecordLock() {
        DeleteCriticalSection(&cs);
    }

    RecordLock(const RecordLock&) = delete;
    RecordLock& operator=(const RecordLock&) = delete;

    void lock_read() {
        EnterCriticalSection(&cs);
        while (writer) {
            LeaveCriticalSection(&cs);
            Sleep(1);
            EnterCriticalSection(&cs);
        }
        readers++;
        LeaveCriticalSection(&cs);
    }

    void unlock_read() {
        EnterCriticalSection(&cs);
        readers--;
        LeaveCriticalSection(&cs);
    }

    void lock_write() {
        EnterCriticalSection(&cs);
        while (writer || readers > 0) {
            LeaveCriticalSection(&cs);
            Sleep(1);
            EnterCriticalSection(&cs);
        }
        writer = true;
        LeaveCriticalSection(&cs);
    }

    void unlock_write() {
        EnterCriticalSection(&cs);
        writer = false;
        LeaveCriticalSection(&cs);
    }
};

#endif