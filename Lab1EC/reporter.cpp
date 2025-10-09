#include <windows.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include "employee.h"
using namespace std;

bool validateRepArg(int argc, char* argv[], string& binfile, string& reportfile, int& pay_per_hour) {
    if (argc < 4) {
        cout << "Usage: Reporter <bin_file> <report_file> <pay_per_hour>\n";
        return false;
    }
    binfile = argv[1];
    reportfile = argv[2];
    try {
        pay_per_hour = stoi(argv[3]);
        if (pay_per_hour <= 0) {
            throw invalid_argument("Pay per hour must be positive!");
        }
    }
    catch (const exception& e) {
        cerr << "Error: Invalid pay per hour value. " << e.what() << endl;
        return false;
    }
    return true;
}

bool processEmployee(ifstream& fin, ofstream& fout, int pay_per_hour) {

    fout << setw(10) << "ID" << setw(15) << "Name" << setw(10) << "Hours" << setw(15) << "Salary" << endl;
    fout << string(50, '-') << endl;

    employee emp;
    bool success = true;

    while (emp.readFromFile(fin)) {
        double salary = emp.hours * pay_per_hour;
        fout << setw(10) << emp.num << setw(15) << emp.name
            << setw(10) << fixed << setprecision(2) << emp.hours
            << setw(15) << fixed << setprecision(2) << salary << endl;

        if (!fout.good()) {
            cerr << "Error writing to report file!" << endl;
            success = false;
            break;
        }
    }

    if (fin.bad()) {
        cerr << "Error reading binary file!" << endl;
        success = false;
    }

    return success;
}

bool openBinRepFile(const string& binfile, const string& reportfile, ifstream& fin, ofstream& fout) {
    fin.open(binfile, ios::binary);
    if (!fin.is_open()) {
        cerr << "Can't open binary file: " << binfile << endl;
        return false;
    }

    fout.open(reportfile);
    if (!fout) {
        cerr << "Can't open report file: " << reportfile << endl;
        fin.close();
        return false;
    }
    return true;
}

int runReporter(const string& binfile, const string& reportfile, int pay_per_hour) {
    ifstream fin;
    ofstream fout;

    if (!openBinRepFile(binfile, reportfile, fin, fout)) {
        return 2;
    }

    if (!processEmployee(fin, fout, pay_per_hour)) {
        fin.close();
        fout.close();
        return 3;
    }

    fin.close();
    fout.close();
    return 0;
}

int main(int argc, char* argv[]) {
    string binfile, reportfile;
    int pay_per_hour;

    if (!validateRepArg(argc, argv, binfile, reportfile, pay_per_hour)) { 
        return 1; 
    }

    return runReporter(binfile, reportfile, pay_per_hour);
}
