#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>

using namespace std;

const int MAX_MESSAGE_LENGTH = 20;
const DWORD WAIT_TIMEOUT_MS = 1000;

enum ConsoleColor {
    GREEN = 2,
    YELLOW = 6,
    RED = 4,
    WHITE = 7
};

void SetColor(ConsoleColor color) {
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdOut, (WORD)color);
}

class Sender {
private:
    string filename;
    int message_count;
    HANDLE hReadyEvent;
    HANDLE hMutex;
    HANDLE hEmptySemaphore;
    HANDLE hFullSemaphore;

    fstream file;
    int write_index = 0;
    bool should_exit = false;

public:
    Sender(const string& fname, const string& eventName,
        const string& mutexName, const string& emptyName,
        const string& fullName, int count)
        : filename(fname), message_count(count) {

        hReadyEvent = OpenEventA(EVENT_MODIFY_STATE, FALSE, eventName.c_str());
        hMutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, mutexName.c_str());
        hEmptySemaphore = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, emptyName.c_str());
        hFullSemaphore = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, fullName.c_str());

        if (!hReadyEvent || !hMutex || !hEmptySemaphore || !hFullSemaphore) {
            SetColor(RED);
            throw runtime_error("Sync objects error");
        }
    }

    ~Sender() {
        CloseHandle(hReadyEvent);
        CloseHandle(hMutex);
        CloseHandle(hEmptySemaphore);
        CloseHandle(hFullSemaphore);
    }

    void signalReady() {
        SetEvent(hReadyEvent);
    }

    void run() {
        signalReady();
        SetColor(GREEN);
        cout << "Sender ready (buffer: " << message_count << ")" << endl;
        SetColor(WHITE);

        while (!should_exit) {
            cout << "\nCommand (send/exit): ";
            string command;

            MSG msg;
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_QUIT) {
                    break;
                }
            }

            if (!getline(cin, command)) {
                continue;
            }

            if (command == "send") {
                if (!sendMessage()) {
                    SetColor(YELLOW);
                    cout << "Buffer full" << endl;
                    SetColor(WHITE);
                }
            }
            else if (command == "exit") {
                break;
            }
            else {
                SetColor(YELLOW);
                cout << "Unknown command" << endl;
                SetColor(WHITE);
            }
        }

        SetColor(GREEN);
        cout << "Sender exiting..." << endl;
        SetColor(WHITE);
    }

private:
    bool sendMessage() {
        string message;
        cout << "Message (" << MAX_MESSAGE_LENGTH << " chars max): ";
        getline(cin, message);

        if (message.length() > MAX_MESSAGE_LENGTH) {
            SetColor(YELLOW);
            cout << "Too long, truncating..." << endl;
            SetColor(WHITE);
            message = message.substr(0, MAX_MESSAGE_LENGTH);
        }

        DWORD result = WaitForSingleObject(hEmptySemaphore, WAIT_TIMEOUT_MS);
        if (result == WAIT_TIMEOUT) {
            return false; 
        }
        else if (result != WAIT_OBJECT_0) {
            SetColor(RED);
            cerr << "Semaphore error" << endl;
            SetColor(WHITE);
            return false;
        }

        WaitForSingleObject(hMutex, INFINITE);

        file.open(filename, ios::binary | ios::in | ios::out);
        if (!file.is_open()) {
            SetColor(RED);
            cerr << "File error" << endl;
            SetColor(WHITE);
            ReleaseMutex(hMutex);
            return false;
        }

        char buffer[MAX_MESSAGE_LENGTH] = { 0 };
        int slotIndex = -1;

        for (int i = 0; i < message_count; i++) {
            int index = (write_index + i) % message_count;
            file.seekg(index * MAX_MESSAGE_LENGTH);
            file.read(buffer, MAX_MESSAGE_LENGTH);

            bool isEmpty = true;
            for (int j = 0; j < MAX_MESSAGE_LENGTH; j++) {
                if (buffer[j] != 0) {
                    isEmpty = false;
                    break;
                }
            }

            if (isEmpty) {
                slotIndex = index;
                file.seekp(index * MAX_MESSAGE_LENGTH);
                file.write(message.c_str(), MAX_MESSAGE_LENGTH);
                write_index = (index + 1) % message_count;
                break;
            }
        }

        file.close();
        ReleaseMutex(hMutex);

        if (slotIndex != -1) {
            SetColor(GREEN);
            cout << "Sent to slot " << slotIndex << endl;
            SetColor(WHITE);
            ReleaseSemaphore(hFullSemaphore, 1, NULL);
            return true;
        }
        else {
            ReleaseSemaphore(hEmptySemaphore, 1, NULL);
            return false;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 7) {
        SetColor(RED);
        cerr << "Usage: sender.exe <filename> <readyEventName> <mutexName>" << endl;
        cerr << "                  <emptySemName> <fullSemName> <messageCount>" << endl;
        SetColor(WHITE);
        return 1;
    }

    string filename = argv[1];
    string eventName = argv[2];
    string mutexName = argv[3];
    string emptyName = argv[4];
    string fullName = argv[5];
    int messageCount = stoi(argv[6]);

    try {
        Sender sender(filename, eventName, mutexName, emptyName, fullName, messageCount);
        sender.run();
    }
    catch (const exception& e) {
        SetColor(RED);
        cerr << "Error: " << e.what() << endl;
        SetColor(WHITE);
        return 1;
    }

    return 0;
}