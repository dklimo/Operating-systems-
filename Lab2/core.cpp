#include "core.h"
#include <iostream>
#include <windows.h>

using namespace std;

CalculationResults g_results;

vector<int> replace_min_max_with_average(const vector<int>& arr, const CalculationResults& results) {
    vector<int> result = arr;
    int avg = static_cast<int>(results.average_val);

    for (int& val : result) {
        if (val == results.min_val || val == results.max_val) {
            val = avg;
        }
    }

    return result;
}

DWORD WINAPI min_max_thread(LPVOID lpParam) {
    vector<int>* arr = static_cast<vector<int>*>(lpParam);
    auto result = find_min_max(*arr);
    g_results.min_val = result.first;
    g_results.max_val = result.second;
    cout << "Thread min_max: min = " << g_results.min_val << ", max = " << g_results.max_val << endl;
    return 0;
}

DWORD WINAPI average_thread(LPVOID lpParam) {
    vector<int>* arr = static_cast<vector<int>*>(lpParam);
    g_results.average_val = find_average(*arr);
    cout << "Thread average: avg = " << g_results.average_val << endl;
    return 0;
}