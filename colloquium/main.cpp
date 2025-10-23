#include <iostream>
#include "exesizes.h"

int main() {
    int n1, n2;
    cout << "Enter Fibonacci count: ";
    cin >> n1;
    cout << "Enter number to check palindrome: ";
    cin >> n2;
    ListN* head = inputList();
    PrintResult(n1, n2, head); 
	return 0;
}