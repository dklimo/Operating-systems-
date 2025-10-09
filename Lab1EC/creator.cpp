#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include "employee.h"
using namespace std;

bool validateCreArg(int argc, char* argv[], string& filename, int& count) {
    if (argc != 3) {
        cerr << "Usage: Creator <filename> <record_count>" << endl;
        return false;
    }

    filename = argv[1];
    try {
        count = stoi(argv[2]);
        if (count <= 0) { 
            throw invalid_argument("Pay per hour must be positive!");
        }
    }
    catch (const exception& e) {
        cerr << "Error: Invalid pay per hour value. " << e.what() << endl;
        return false;
    }
    return true;
}
bool writeRepFile(const string& filename, int count) {
    ofstream fout(filename, ios::binary);
    if (!fout) {
        cerr << "Error creating file: " << filename << endl;
        return false;
    }

    cout << "Enter " << count << " employee records:" << endl;

    for (int i = 0; i < count; i++) {
        employee emp;
        cout << "Employee number " << (i + 1) << ":" << endl;

        cout << "Enter ID: ";
        cin >> emp.num;

        cout << "Enter name: ";
        cin.ignore(); 
        cin.getline(emp.name, sizeof(emp.name));

        cout << "Enter hours worked: ";
        cin >> emp.hours;

        if (!emp.writeToFile(fout)) {
            cerr << "Error writing employee data to file!" << endl;
            fout.close();
            return false;
        }
        cout << endl;
    }

    fout.close();
    return true;
}
int runCreator(const string& filename, int count) {
    ifstream fin;
    ofstream fout;

    if (!writeRepFile(filename, count)) {
        return 2;
    }
    fin.close();
    fout.close();
    return 0;
}
int main(int argc, char* argv[]) {

    string filename;
    int count;
    if (!validateCreArg(argc, argv, filename, count)) { 
        return 1;
    }

    return runCreator(filename, count); 
}
