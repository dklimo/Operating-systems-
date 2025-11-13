#include <iostream>
#include "helper.h"  

using namespace helper;  
int GetValidatedArraySize() {
    int size;
    while (true) {
        cout << "Enter array size: ";
        cin >> size;

        if (cin.fail() || size <= 0) {
            cin.clear();
            cin.ignore(INPUT_BUFFER_CLEAR_SIZE, '\n');
            cout << "Invalid array size! Please enter a positive integer." << endl;
        }
        else {
            return size;
        }
    }
}

int GetValidatedThreadCount() {
    int count;
    while (true) {
        cout << "Enter number of marker threads: ";
        cin >> count;

        if (cin.fail() || count <= 0) {
            cin.clear();
            cin.ignore(INPUT_BUFFER_CLEAR_SIZE, '\n'); 
            cout << "Invalid thread count! Please enter a positive integer." << endl;
        }
        else {
            return count;
        }
    }
}
