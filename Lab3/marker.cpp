#include "marker.h"

struct ThreadParams {
    MarkerManager* manager;
    int threadId;
};

MarkerManager::MarkerManager()
    : threadCount(0), activeThreads(0), initialized(false) {
}

MarkerManager::~MarkerManager() {
    Cleanup();
}

bool MarkerManager::Initialize(int arraySize, int markerThreadCount) {
    if (arraySize <= 0 || markerThreadCount <= 0) {
        cout << "Invalid parameters" << endl;
        return false;
    }

    array.resize(arraySize, 0);
    threadCount = markerThreadCount;
    activeThreads = markerThreadCount;

    if (!sync.Initialize(threadCount)) {
        cout << "Failed to initialize synchronization" << endl;
        return false;
    }

    threadActive.resize(threadCount, true);
    threads.resize(threadCount);

    if (!CreateMarkerThreads()) {
        cout << "Failed to create marker threads" << endl;
        return false;
    }

    initialized = true;
    return true;
}

bool MarkerManager::CreateMarkerThreads() {
    for (int i = 0; i < threadCount; i++) {
        ThreadParams* params = new ThreadParams{ this, i + 1 };
        threads[i] = CreateThread(NULL, 0, MarkerThread, params, 0, NULL);

        if (threads[i] == NULL) {
            cout << "Failed to create thread " << (i + 1) << endl;
            delete params;
            return false;
        }
    }
    return true;
}

void MarkerManager::Run() {
    if (!initialized) {
        cout << "Manager not initialized" << endl;
        return;
    }

    SignalStart();
    while (activeThreads > 0) {
        ResetEvent(sync.GetContinueEvent());
        while (!AllThreadsWaiting()) {
            Sleep(THREAD_SLEEP_WAIT);
        }

        cout << "\nAll threads are waiting." << endl;
        PrintArray();

        cout << "Active streams: ";
        for (int i = 0; i < threadCount; ++i) {
            if (threadActive[i]) cout << (i + 1) << " ";
        }
        cout << endl;

        int threadToTerminate = -1;
        while (true) {
            cout << "Please enter the stream number to complete (1-" << threadCount << "): ";
            cin >> threadToTerminate;

            if (threadToTerminate < 1 || threadToTerminate > threadCount) {
                cout << "Invalid stream number! Please try again." << endl;
                continue;
            }

            if (!threadActive[threadToTerminate - 1]) {
                cout << "Stream " << threadToTerminate << " has already ended. Please choose another." << endl;
                continue;
            }

            break;
        }

        TerminateThread(threadToTerminate - 1);

        cout << "After the thread completes " << threadToTerminate << ":" << endl;
        PrintArray();

        if (activeThreads > 0) {
            SignalContinue();
        }
    }

    cout << "All threads have completed their work." << endl;
}


void MarkerManager::SignalStart() {
    SetEvent(sync.GetStartEvent());
}

void MarkerManager::SignalContinue() {
    for (int i = 0; i < threadCount; i++) {
        if (threadActive[i]) {
            sync.ResetWaitEvent(i);
        }
    }
    SetEvent(sync.GetContinueEvent());
}

bool MarkerManager::AllThreadsWaiting() const {
    for (int i = 0; i < threadCount; i++) {
        if (threadActive[i] && !sync.IsWaitEventSet(i)) {
            return false;
        }
    }
    return true;
}

void MarkerManager::TerminateThread(int threadIndex) {
    SetEvent(sync.GetTerminateEvent(threadIndex));
    WaitForSingleObject(threads[threadIndex], INFINITE);
    threadActive[threadIndex] = false;
    activeThreads--;
}

void MarkerManager::PrintArray() const {
    cout << "Array: ";
    for (size_t i = 0; i < array.size(); i++) {
        cout << array[i];
        if (i < array.size() - 1) {
            cout << ", ";
        }
    }
    cout << endl;
}

void MarkerManager::Cleanup() {
    for (auto& thread : threads) {
        if (thread) {
            CloseHandle(thread);
        }
    }
    threads.clear();
    sync.Cleanup();
    initialized = false;
}

DWORD WINAPI MarkerManager::MarkerThread(LPVOID lpParam) {
    ThreadParams* params = static_cast<ThreadParams*>(lpParam);
    params->manager->MarkerThreadMethod(params->threadId);
    delete params;
    return SUCCESS_EXIT;
}

void MarkerManager::MarkerThreadMethod(int threadId) {
    WaitForSingleObject(sync.GetStartEvent(), INFINITE);
    srand(threadId);

    vector<int> markedIndices;
    bool working = true;
    int threadIndex = threadId - 1;
    int marksCount = 0;

    while (working) {
        int index = rand() % array.size();

        sync.EnterCriticalSection();
        if (array[index] == 0) {
            Sleep(THREAD_SLEEP);
            array[index] = threadId;
            markedIndices.push_back(index);
            marksCount++;
            Sleep(THREAD_SLEEP);
            sync.LeaveCriticalSection();
        }
        else {
            const string colors[] = { "\033[31m",  "\033[32m", "\033[33m", "\033[34m",  "\033[35m",  "\033[36m", "\033[91m", "\033[92m", "\033[93m", "\033[94m", };
            string color = colors[(threadId - 1) % (sizeof(colors) / sizeof(colors[0]))];
            string reset = "\033[0m";
            cout << color
                << "Thread " << threadId << ": marked " << marksCount
                << " elements, cannot mark index " << index
                << reset << endl;
            sync.SetWaitEvent(threadIndex);
            sync.LeaveCriticalSection();

            HANDLE hEvents[2] = {
                sync.GetContinueEvent(),
                sync.GetTerminateEvent(threadIndex)
            };

            DWORD dwEvent = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);

            if (dwEvent == WAIT_OBJECT_0 + 1) {
                working = false;
            }
        }
        if (marksCount % FORCED_WAIT == 0 && marksCount > 0) { 
            sync.EnterCriticalSection();
            const string colors[] = { "\033[31m",  "\033[32m", "\033[33m", "\033[34m",  "\033[35m",  "\033[36m", "\033[91m", "\033[92m", "\033[93m", "\033[94m", };
            string color = colors[(threadId - 1) % (sizeof(colors) / sizeof(colors[0]))];
            string reset = "\033[0m";
            cout << color
                << "Thread " << threadId << ": forced wait after " << marksCount << " marks"
                << reset << endl;
            sync.SetWaitEvent(threadIndex);
            sync.LeaveCriticalSection();

            HANDLE hEvents[2] = {
                sync.GetContinueEvent(),
                sync.GetTerminateEvent(threadIndex)
            };

            DWORD dwEvent = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);

            if (dwEvent == WAIT_OBJECT_0 + 1) {
                working = false;
            }
        }
    }

    sync.EnterCriticalSection();
    for (int idx : markedIndices) {
        if (array[idx] == threadId) {
            array[idx] = 0;
        }
    }
    sync.LeaveCriticalSection();
}
