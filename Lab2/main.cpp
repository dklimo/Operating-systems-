#include <windows.h>
#include <iostream>
#include <vector>
#include "min_max.h"
using namespace std;
DWORD WINAPI min_max_thread(LPVOID lpParam) {
    vector<int>* arr = static_cast<vector<int>*>(lpParam);
    auto result = find_min_max(*arr);
    cout << "Thread min_max: min = " << result.first << ", max = " << result.second << endl;
    return 0;
}

int main() {
    int n;
    cout << "Enter array size: ";
    cin >> n;

    vector<int> A(n);
    cout << "Enter array members: ";
    for (int i = 0; i < n; ++i) {
        cin >> A[i];
    }

    DWORD IDMinMax;
    HANDLE hMinMax = CreateThread(NULL, 0, min_max_thread, &A, 0, &IDMinMax);
    if (hMinMax == NULL) {
        cerr << "Error creating thread: " << GetLastError() << endl;
        return 1;
    }

    WaitForSingleObject(hMinMax, INFINITE);
    CloseHandle(hMinMax);

    return 0;
}
