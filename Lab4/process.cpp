#include "process.h"
#include <windows.h>
#include <vector>
#include <string>

using namespace std;

bool processLauncher(vector<char>& cmdLine, PROCESS_INFORMATION& pi) {
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (CreateProcessA(
        NULL,
        cmdLine.data(),
        NULL,
        NULL,
        FALSE,
        CREATE_NEW_CONSOLE,
        NULL,
        NULL,
        &si,
        &pi
    )) {
        return true;
    }
    else {
        return false;
    }
}

HANDLE createReadyEvent(const string& baseName, int index) {
    string eventName = baseName + "_ready_" + to_string(index + 1);
    HANDLE hEvent = CreateEventA(NULL, FALSE, FALSE, eventName.c_str());
    return hEvent;
}

bool launchSenderProcess(const string& command, PROCESS_INFORMATION& pi) {
    vector<char> cmdbuf(command.begin(), command.end());
    cmdbuf.push_back('\0');
    return processLauncher(cmdbuf, pi);
}