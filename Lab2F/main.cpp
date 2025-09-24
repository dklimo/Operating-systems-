#include <windows.h>
#include <iostream>
#include <vector>
#include "min_max.h"
#include "average.h" 
using namespace std;
double glaverage = 0;
int glmin = 0, glmax = 0;

DWORD WINAPI min_max_thread(LPVOID lpParam) {
    vector<int>* arr = static_cast<vector<int>*>(lpParam);
    auto result = find_min_max(*arr);
    glmin = result.first;
    glmax = result.second; 
    cout << "Thread min_max: min = " << glmin << ", max = " << glmax << endl;
    return 0;
}
DWORD WINAPI average_thread(LPVOID lpParam) {
    vector<int>* arr = static_cast<vector<int>*>(lpParam);
    glaverage = find_average(*arr);
    cout << "Thread average: avg = " << glaverage << endl;
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

    DWORD IDMinMax, IDAverage; 
    HANDLE hMinMax = CreateThread(NULL, 0, min_max_thread, &A, 0, &IDMinMax);
    HANDLE hAverage = CreateThread(NULL, 0, average_thread, &A, 0, &IDAverage);
    if (!hMinMax || !hAverage) { 
        cerr << "Error creating threads: " << GetLastError() << endl; 
        return 1;
    }
    WaitForSingleObject(hMinMax, INFINITE);
    WaitForSingleObject(hAverage, INFINITE); 

    for (int& val : A) {
        if (val == glmin || val == glmax) {
            val = static_cast<int>(glaverage);
        }
    }

    cout << "Modified array: ";
    for (int val : A) {
        cout << val << " ";
    }
    cout << endl;
    CloseHandle(hMinMax);
    CloseHandle(hAverage); 
    return 0;
}
