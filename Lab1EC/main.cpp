#include <windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "employee.h"
using namespace std;

int main() {
    string binaryfile, reportfile;
    int recordcount;
    double pay_per_hour;
    cout << "Enter binary file name (.bin): ";
    cin >> binaryfile;
    cout << "Enter number of records: ";
    cin >> recordcount;

    vector<char> cmdLine1;
    string creatorCommand = string("Creator.exe") + ' ' + binaryfile + ' ' + to_string(recordcount);
    cmdLine1.assign(creatorCommand.begin(), creatorCommand.end());
    cmdLine1.push_back('\0');

    STARTUPINFOA si1;
    PROCESS_INFORMATION pi1;
    ZeroMemory(&si1, sizeof(si1));
    si1.cb = sizeof(si1);
    ZeroMemory(&pi1, sizeof(pi1));

    if (!CreateProcessA(NULL, cmdLine1.data(), NULL, NULL, FALSE, 0, NULL, NULL, &si1, &pi1)) {
        cerr << "Failed to start Creator.exe. Error: " << GetLastError() << "\n";
        return 1;
    }
    WaitForSingleObject(pi1.hProcess, INFINITE);
    CloseHandle(pi1.hProcess);
    CloseHandle(pi1.hThread);

    writeBinaryFile(binaryfile);

    cout << "Enter report file name (.txt): ";
    cin >> reportfile;
    cout << "Enter pay per hour: ";
    cin >> pay_per_hour;

    vector<char> cmdLine2;
    string reporterCommand = string("Reporter.exe") + ' ' + binaryfile + ' ' + reportfile + ' ' + to_string(pay_per_hour);
    cmdLine2.assign(reporterCommand.begin(), reporterCommand.end());
    cmdLine2.push_back('\0');

    STARTUPINFOA si2;
    PROCESS_INFORMATION pi2;
    ZeroMemory(&si2, sizeof(si2));
    si2.cb = sizeof(si2);
    ZeroMemory(&pi2, sizeof(pi2));

    if (!CreateProcessA(NULL, cmdLine2.data(), NULL, NULL, FALSE, 0, NULL, NULL, &si2, &pi2)) {
        cerr << "Failed to start Reporter.exe. Error: " << GetLastError() << "\n";
        return 1;
    }

    WaitForSingleObject(pi2.hProcess, INFINITE);
    CloseHandle(pi2.hProcess);
    CloseHandle(pi2.hThread);

    writeReport(reportfile);

    return 0;
}