#include "employee.h"
#include <fstream>
#include <iostream>
#include <string>
using namespace std;

void writeBinaryFile(const string& filename) {
    ifstream file(filename, ios::binary);
    if (!file) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    cout << "\nContents of binary file '" << filename << "':" << endl;

    employee emp;
    while (file.read(reinterpret_cast<char*>(&emp), sizeof(employee))) {
        cout << "ID: " << emp.num << ", Name: " << emp.name
            << ", Hours: " << emp.hours << endl;
    }

    file.close();
}

void writeReport(const string& filename) {
    ifstream file(filename);
    if (!file) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    cout << "\nContents of report file '" << filename << "':" << endl;
    string line;
    while (getline(file, line)) {
        cout << line << endl;
    }

    file.close();
}
