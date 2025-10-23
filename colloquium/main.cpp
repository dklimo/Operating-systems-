#include <iostream>
#include "exesizes.h"

int main() {
    try {
        int n1, n2;
        cout << "Enter Fibonacci count: ";
        if (!(cin >> n1)) {
            throw invalid_argument("Invalid Fibonacci count input");
        }
        cout << "Enter number for palindrome check: ";
        if (!(cin >> n2)) {
            throw invalid_argument("Invalid palindrome number input");
        }

        ListN* head = inputList();
        PrintResult(n1, n2, head);

    }
    catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
