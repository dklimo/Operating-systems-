#include <windows.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>

using namespace std;

struct employee {
    int num;
    char name[10];
    double hours;
};

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cout << "Usage: Reporter <bin_file> <report_file> <pay_per_hour>\n";
        return 1;
    }
    string binfile = argv[1];
    string reportfile = argv[2];
    int pay_per_hour = stoi(argv[3]);

    ifstream fin(binfile, ios::binary);
    if (!fin.is_open()) {
        cerr << "Can't open binary file.\n";
        return 2;
    }
    ofstream fout(reportfile);
    if (!fout) {
        cerr << "Can't open report file.\n";
        return 3;
    }

    fout << setw(10) << "ID" << setw(15) << "Name" << setw(10) << "Hours" << setw(15) << "Salary" << endl;
    fout << string(50, '-') << endl;

    employee emp;
    while (fin.read(reinterpret_cast<char*>(&emp), sizeof(emp))) {
        double salary = emp.hours * pay_per_hour;
        fout << setw(10) << emp.num << setw(15) << emp.name
            << setw(10) << fixed << setprecision(2) << emp.hours
            << setw(15) << fixed << setprecision(2) << salary << endl;
    }
    fin.close();
    fout.close();
    return 0;
}