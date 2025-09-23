#include "min_max.h"
#include <windows.h>
#include <limits>

using namespace std;

pair<int, int> find_min_max(const std::vector<int>& arr) {
    if (arr.empty()) return { INT_MAX, INT_MIN };

    int min = arr[0];
    int max = arr[0];

    for (size_t i = 1; i < arr.size(); ++i) {
        if (arr[i] < min) min = arr[i];
        Sleep(7);
        if (arr[i] > max) max = arr[i];
        Sleep(7);
    }

    return { min, max };
}
