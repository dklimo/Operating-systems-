#ifndef PROCESS_H
#define PROCESS_H

#include <windows.h>
#include <string>
#include <vector>
using namespace std;
bool processLauncher(vector<char>& cmdLine, PROCESS_INFORMATION& pi);
HANDLE createReadyEvent(const string& baseName, int index);
bool launchSenderProcess(const string& command, PROCESS_INFORMATION& pi); 

#endif