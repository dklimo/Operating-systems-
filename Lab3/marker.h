#ifndef MARKER_H
#define MARKER_H

#include <windows.h>
#include "threadsync.h"
#include "helper.h" 

using namespace helper; 

class MarkerManager {
public:
    MarkerManager(); 
    ~MarkerManager();

    bool Initialize(int arraySize, int markerThreadCount);
    void Run();
    void Cleanup();
    void PrintArray() const;
    int GetActiveThreadCount() const { return activeThreads; }

private:
    vector<int> array; 
    vector<HANDLE> threads;
    vector<bool> threadActive;
    ThreadSync sync;

    int threadCount;
    int activeThreads;
    bool initialized;

    bool CreateMarkerThreads();
    void SignalStart();
    bool AllThreadsWaiting() const;
    void TerminateThread(int threadIndex);
    void SignalContinue();
    static DWORD WINAPI MarkerThread(LPVOID lpParam);

    void MarkerThreadMethod(int threadId);
};

#endif