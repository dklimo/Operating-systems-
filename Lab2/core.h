#pragma once

#include <vector>
#include <utility>
#include <windows.h> 
#include "min_max.h"
#include "average.h"

using namespace std;
struct CalculationResults {
    int min_val = 0;
    int max_val = 0;
    double average_val = 0.0;

    CalculationResults(int min = 0, int max = 0, double avg = 0.0)
        : min_val(min), max_val(max), average_val(avg) {}
};

vector<int> replace_min_max_with_average(const std::vector<int>& arr, const CalculationResults& results);

DWORD WINAPI min_max_thread(LPVOID lpParam);
DWORD WINAPI average_thread(LPVOID lpParam);

extern CalculationResults g_results;