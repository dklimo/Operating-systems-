#include "threadsync.h"
#include <iostream>

ThreadSync::ThreadSync()
    : hStartEvent(NULL), hContinueEvent(NULL), threadCount(0) {
    InitializeCriticalSection(&csArray);
}

ThreadSync::~ThreadSync() {
    Cleanup();
    DeleteCriticalSection(&csArray);
}

bool ThreadSync::Initialize(int count) {
    threadCount = count;

    hStartEvent = CreateEvent(NULL, TRUE, FALSE, NULL);     
    hContinueEvent = CreateEvent(NULL, TRUE, FALSE, NULL);  

    if (!hStartEvent || !hContinueEvent) {
        std::cout << "Failed to create start/continue events" << std::endl;
        return false;
    }

    hWaitEvents.resize(threadCount);
    hTerminateEvents.resize(threadCount);

    for (int i = 0; i < threadCount; i++) {
        hWaitEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);     
        hTerminateEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL); 

        if (!hWaitEvents[i] || !hTerminateEvents[i]) {
            std::cout << "Failed to create events for thread " << i << std::endl;
            return false;
        }
    }

    return true;
}

void ThreadSync::Cleanup() {
    if (hStartEvent) CloseHandle(hStartEvent);
    if (hContinueEvent) CloseHandle(hContinueEvent);

    for (auto& e : hWaitEvents) {
        if (e) CloseHandle(e);
    }
    for (auto& e : hTerminateEvents) {
        if (e) CloseHandle(e);
    }

    hWaitEvents.clear();
    hTerminateEvents.clear();

    hStartEvent = hContinueEvent = NULL;
}

bool ThreadSync::IsWaitEventSet(int index) const {
    return WaitForSingleObject(hWaitEvents[index], 0) == WAIT_OBJECT_0;
}
