#include <iostream>
#include <windows.h>
#include "marker.h" 

int main() {
    MarkerManager manager;
    int arraySize = GetValidatedArraySize(); 
    int threadCount = GetValidatedThreadCount();  

    if (!manager.Initialize(arraySize, threadCount)) {
        cout << "Failed to initialize marker manager" << endl;
        return FAILURE_EXIT;
    }

    manager.Run();
    manager.Cleanup();

    cout << "Main thread exiting." << endl;
    return SUCCESS_EXIT;
}
