#ifndef THREADSYNC_H
#define THREADSYNC_H

#include <windows.h>
#include "helper.h" 

using namespace helper; 
class ThreadSync {
public:
    ThreadSync();
    ~ThreadSync();

    bool Initialize(int threadCount);
    void Cleanup();

    HANDLE GetStartEvent() const { return hStartEvent; }
    HANDLE GetContinueEvent() const { return hContinueEvent; }
    HANDLE GetTerminateEvent(int index) const { return hTerminateEvents[index]; }
    HANDLE GetWaitEvent(int index) const { return hWaitEvents[index]; }

    void EnterCriticalSection() { ::EnterCriticalSection(&csArray); }
    void LeaveCriticalSection() { ::LeaveCriticalSection(&csArray); }

    void SetWaitEvent(int index) { SetEvent(hWaitEvents[index]); }
    void ResetWaitEvent(int index) { ResetEvent(hWaitEvents[index]); }
    bool IsWaitEventSet(int index) const;

private:
    HANDLE hStartEvent;
    HANDLE hContinueEvent;
    vector<HANDLE> hTerminateEvents;
    vector<HANDLE> hWaitEvents;
    CRITICAL_SECTION csArray;
    int threadCount;
};

#endif
