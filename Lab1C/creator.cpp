#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

struct employee {
    int num;
    char name[10];
    double hours;
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: Creator <filename> <record_count>" << endl;
        return 1;
    }

    string filename = argv[1];
    int count = stoi(argv[2]);

    ofstream fout(filename, ios::binary);
    if (!fout) {
        cerr << "Error creating file: " << filename << endl;
        return 1;
    }

    cout << "Enter " << count << " employee records:" << endl;

    for (int i = 0; i < count; i++) {
        employee emp;
        cout << "Employee number " << (i + 1) << ":" << endl;

        cout << "Enter ID: ";
        cin >> emp.num;

        cout << "Enter name: ";
        cin >> emp.name;

        cout << "Enter hours worked: ";
        cin >> emp.hours;

        fout.write(reinterpret_cast<char*>(&emp), sizeof(employee));
        cout << endl; 
    }

    fout.close();

    return 0;
}