#include <windows.h>
#include <iostream>
#include <vector>
#include "core.h"

using namespace std;

int main() {
    int n;
    cout << "Enter array size: ";
    cin >> n;

    vector<int> A(n);
    cout << "Enter array members: ";
    for (int i = 0; i < n; ++i) {
        cin >> A[i];
    }

    DWORD IDMinMax, IDAverage; 
    HANDLE hMinMax = CreateThread(NULL, 0, min_max_thread, &A, 0, &IDMinMax);
    HANDLE hAverage = CreateThread(NULL, 0, average_thread, &A, 0, &IDAverage);
    if (!hMinMax || !hAverage) { 
        cerr << "Error creating threads: " << GetLastError() << endl; 
        return 1;
    }
    WaitForSingleObject(hMinMax, INFINITE);
    WaitForSingleObject(hAverage, INFINITE);

    vector<int> modified_array = replace_min_max_with_average(A, g_results); 

    cout << "Modified array: ";
    for (int val : modified_array) {
        cout << val << " ";
    }
    cout << endl;

    CloseHandle(hMinMax);
    CloseHandle(hAverage);

    return 0;
}



