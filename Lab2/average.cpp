#include "average.h"
#include <windows.h>

using namespace std;
const int AVERAGE_CALCULATE_DELAY = 12;
double find_average(const vector<int>& arr) {
	if (arr.empty()) return 0.0;
	long long sum = 0;
	for (int value : arr) {
		sum += value; 
		Sleep(AVERAGE_CALCULATE_DELAY);
	}
	return static_cast<double>(sum) / arr.size();
}
