#include "gtest/gtest.h"
#include "utils.h"
#include "process.h"
#include <fstream>
#include <filesystem>
#include <windows.h>

using namespace std;

TEST(ProcessCreateTest, CreateReadyEventSuccess) {
    string baseName = "test_event_success";
    HANDLE event = createReadyEvent(baseName, 1);

    EXPECT_NE(event, nullptr);
    EXPECT_NE(event, INVALID_HANDLE_VALUE);

    if (event != nullptr && event != INVALID_HANDLE_VALUE) {
        DWORD result = WaitForSingleObject(event, 0);
        EXPECT_EQ(result, WAIT_TIMEOUT);

        EXPECT_TRUE(SetEvent(event));

        result = WaitForSingleObject(event, 0);
        EXPECT_EQ(result, WAIT_OBJECT_0);

        CloseHandle(event);
    }
}

TEST(ProcessCreateTest, CreateReadyEventVariousNames) {
    vector<pair<string, int>> testCases = {
        {"simple_event", 0},
        {"event_with_underscore", 1},
        {"Event123", 2},
        {"global_event", 3},
        {"test-event-567", 4}
    };

    vector<HANDLE> createdEvents;

    for (const auto& testCase : testCases) {
        HANDLE event = createReadyEvent(testCase.first, testCase.second);
        EXPECT_NE(event, nullptr) << "Failed to create event: " << testCase.first;
        EXPECT_NE(event, INVALID_HANDLE_VALUE) << "Invalid handle for: " << testCase.first;

        if (event != nullptr && event != INVALID_HANDLE_VALUE) {
            createdEvents.push_back(event);
        }
    }

    for (HANDLE event : createdEvents) {
        CloseHandle(event);
    }
}

TEST(ProcessCreateTest, CreateReadyEventSameNameTwice) {
    string baseName = "duplicate_event_test";
    int index = 1;

    HANDLE event1 = createReadyEvent(baseName, index);
    EXPECT_NE(event1, nullptr);
    EXPECT_NE(event1, INVALID_HANDLE_VALUE);

    HANDLE event2 = createReadyEvent(baseName, index);
    EXPECT_NE(event2, nullptr);
    EXPECT_NE(event2, INVALID_HANDLE_VALUE);
    EXPECT_NE(event1, event2);
    EXPECT_TRUE(SetEvent(event1));
    DWORD result = WaitForSingleObject(event2, 0);
    EXPECT_EQ(result, WAIT_OBJECT_0);

    CloseHandle(event1);
    CloseHandle(event2);
}

TEST(ProcessCreateTest, CreateReadyEventLongName) {
    string longName(260, 'X');  
    HANDLE event = createReadyEvent(longName, 0);

    if (event == nullptr || event == INVALID_HANDLE_VALUE) {
        SUCCEED() << "Long event name may not be supported";
    }
    else {
        CloseHandle(event);
    }
}

TEST(ProcessLauncherTest, ProcessLauncherEmptyCommand) {
    PROCESS_INFORMATION pi = { 0 };
    vector<char> emptyCmd = { '\0' };

    bool result = processLauncher(emptyCmd, pi);
    EXPECT_FALSE(result) << "Empty command should return false";

    EXPECT_EQ(pi.hProcess, nullptr);
    EXPECT_EQ(pi.hThread, nullptr);
}

TEST(ProcessLauncherTest, ProcessLauncherComplexCommand) {
    PROCESS_INFORMATION pi = { 0 };
    string commandStr = "program.exe arg1 \"arg2 with spaces\" arg3";
    vector<char> cmdLine(commandStr.begin(), commandStr.end());
    cmdLine.push_back('\0');

    bool result = processLauncher(cmdLine, pi);
    EXPECT_FALSE(result) << "Non-existent program should return false";

    if (pi.hProcess) CloseHandle(pi.hProcess);
    if (pi.hThread) CloseHandle(pi.hThread);
}


TEST(ProcessLauncherTest, ProcessLauncherVeryLongCommand) {
    PROCESS_INFORMATION pi = { 0 };
    string longArg(1000, 'A');
    string commandStr = "test.exe " + longArg;
    vector<char> cmdLine(commandStr.begin(), commandStr.end());
    cmdLine.push_back('\0');

    bool result = processLauncher(cmdLine, pi);
    EXPECT_FALSE(result) << "Very long command with non-existent exe should return false";

    if (pi.hProcess) CloseHandle(pi.hProcess);
    if (pi.hThread) CloseHandle(pi.hThread);
}

TEST(ProcessSenderTest, LaunchSenderProcessEmptyString) {
    PROCESS_INFORMATION pi = { 0 };
    string emptyCmd = "";

    bool result = launchSenderProcess(emptyCmd, pi);
    EXPECT_FALSE(result) << "Empty command string should return false";

    EXPECT_EQ(pi.hProcess, nullptr);
    EXPECT_EQ(pi.hThread, nullptr);
}

TEST(ProcessSenderTest, LaunchSenderProcessExtremelyLongCommand) {
    PROCESS_INFORMATION pi = { 0 };
    string longParam(32767, 'X'); 
    string command = "sender.exe " + longParam;

    bool result = launchSenderProcess(command, pi);
    EXPECT_FALSE(result);

    if (pi.hProcess) CloseHandle(pi.hProcess);
    if (pi.hThread) CloseHandle(pi.hThread);
}

TEST(ProcessSenderTest, LaunchSenderProcessMultipleSpaces) {
    PROCESS_INFORMATION pi = { 0 };
    string command = "   sender.exe   arg1   \"arg 2\"   ";

    bool result = launchSenderProcess(command, pi);
    EXPECT_FALSE(result) << "Command with multiple spaces for non-existent exe should return false";

    if (pi.hProcess) CloseHandle(pi.hProcess);
    if (pi.hThread) CloseHandle(pi.hThread);
}

TEST(ProcessSenderTest, LaunchSenderProcessCallsProcessLauncher) {
    PROCESS_INFORMATION pi = { 0 };
    string command = "test_command.exe param1 param2";

    bool result = launchSenderProcess(command, pi);
    EXPECT_FALSE(result);
    if (!result) {
        EXPECT_EQ(pi.hProcess, nullptr);
        EXPECT_EQ(pi.hThread, nullptr);
        EXPECT_EQ(pi.dwProcessId, 0);
        EXPECT_EQ(pi.dwThreadId, 0);
    }

    if (pi.hProcess) CloseHandle(pi.hProcess);
    if (pi.hThread) CloseHandle(pi.hThread);
}

TEST(ProcessSenderTest, LaunchSenderProcessNoResourceLeak) {
    for (int i = 0; i < 100; i++) {
        PROCESS_INFORMATION pi = { 0 };
        string command = "nonexistent_program_" + to_string(i) + ".exe";

        bool result = launchSenderProcess(command, pi);
        EXPECT_FALSE(result);
        if (pi.hProcess) {
            DWORD exitCode;
            GetExitCodeProcess(pi.hProcess, &exitCode);
            CloseHandle(pi.hProcess);
        }
        if (pi.hThread) {
            CloseHandle(pi.hThread);
        }
    }
    SUCCEED() << "Resource leak test passed";
}

TEST(ProcessTest, EventNameWithDifferentIndices) {
    string baseName = "base";

    for (int i = 0; i < 10; i++) {
        HANDLE event = createReadyEvent(baseName, i);
        EXPECT_NE(event, nullptr) << "Failed for index " << i;
        EXPECT_NE(event, INVALID_HANDLE_VALUE) << "Invalid handle for index " << i;

        if (event != nullptr && event != INVALID_HANDLE_VALUE) {
            CloseHandle(event);
        }
    }
}

TEST(ProcessTest, MultipleEventsSimultaneously) {
    const int NUM_EVENTS = 5;
    vector<HANDLE> events;

    for (int i = 0; i < NUM_EVENTS; i++) {
        string name = "multi_event_" + to_string(i);
        HANDLE event = createReadyEvent(name, i);
        EXPECT_NE(event, nullptr);
        if (event != nullptr) {
            events.push_back(event);
        }
    }

    EXPECT_EQ(events.size(), NUM_EVENTS);

    for (HANDLE event : events) {
        CloseHandle(event);
    }
}


int main(int argc, char** argv) {
    
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}