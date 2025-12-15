
#include "utils.h"
#include "process.h"

class Receiver {
private:
    string filename;
    int message_count;
    int sender_count;
    HANDLE hMutex;
    HANDLE hEmptySemaphore;
    HANDLE hFullSemaphore;

    fstream file;
    int read_index = 0;

public:
    Receiver(const string& fname, int msg_count, int snd_count)
        : filename(fname), message_count(msg_count), sender_count(snd_count) {

        file.open(filename, ios::binary | ios::out | ios::trunc);
        if (!file.is_open()) {
            SetColor(RED);
            throw runtime_error("Failed to create file");
        }

        char emptyMessage[MAX_MESSAGE_LENGTH] = { 0 };
        for (int i = 0; i < message_count; i++) {
            file.write(emptyMessage, MAX_MESSAGE_LENGTH);
        }
        file.close();

        string mutexName = filename + "_mutex";
        string emptySemName = filename + "_empty";
        string fullSemName = filename + "_full";

        hMutex = CreateMutexA(NULL, FALSE, mutexName.c_str());
        hEmptySemaphore = CreateSemaphoreA(NULL, message_count, message_count, emptySemName.c_str());
        hFullSemaphore = CreateSemaphoreA(NULL, 0, message_count, fullSemName.c_str());

        if (!hMutex || !hEmptySemaphore || !hFullSemaphore) {
            SetColor(RED);
            throw runtime_error("Failed to create synchronization objects");
        }

        SetColor(GREEN);
        cout << "File created: " << filename << " (buffer: " << message_count << " messages)" << endl;
        SetColor(WHITE);
    }

    ~Receiver() {
        CloseHandle(hMutex);
        CloseHandle(hEmptySemaphore);
        CloseHandle(hFullSemaphore);
    }

    void waitForAllSenders() {
        cout << "Waiting for " << sender_count << " senders..." << endl;

        vector<HANDLE> readyEvents(sender_count);
        for (int i = 0; i < sender_count; i++) {
            readyEvents[i] = createReadyEvent(filename, i);
            if (!readyEvents[i]) {
                SetColor(RED);
                throw runtime_error("Failed to create ready event for sender " + to_string(i));
            }
        }

        DWORD result = WaitForMultipleObjects(sender_count, readyEvents.data(), TRUE, INFINITE);
        if (result == WAIT_FAILED) {
            SetColor(RED);
            throw runtime_error("WaitForMultipleObjects failed");
        }

        SetColor(GREEN);
        cout << "All senders ready!" << endl;
        SetColor(WHITE);

        for (int i = 0; i < sender_count; i++) {
            CloseHandle(readyEvents[i]);
        }
    }

    void run() {
        string command;
        while (true) {
            cout << "\nCommand (read/exit): ";
            getline(cin, command);

            if (command == "read") {
                if (!readMessage()) {
                    SetColor(YELLOW);
                    cout << "Buffer is empty" << endl;
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
    }

private:
    bool readMessage() {
        DWORD result = WaitForSingleObject(hFullSemaphore, WAIT_TIMEOUT_MS);
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

        char message[MAX_MESSAGE_LENGTH + 1] = { 0 };
        file.seekg(read_index * MAX_MESSAGE_LENGTH);
        file.read(message, MAX_MESSAGE_LENGTH);
        message[MAX_MESSAGE_LENGTH] = '\0';

        file.seekp(read_index * MAX_MESSAGE_LENGTH);
        char empty[MAX_MESSAGE_LENGTH] = { 0 };
        file.write(empty, MAX_MESSAGE_LENGTH);

        read_index = (read_index + 1) % message_count;

        file.close();
        ReleaseMutex(hMutex);

        SetColor(GREEN);
        cout << "Message: " << message << endl;
        SetColor(WHITE);

        ReleaseSemaphore(hEmptySemaphore, 1, NULL);
        return true;
    }
};

int main() {
    string filename;
    int message_count;
    int sender_count;

    cout << "Binary filename: ";
    getline(cin, filename);

    cout << "Buffer size (messages): ";
    cin >> message_count;
    cin.ignore();

    cout << "Number of senders: ";
    cin >> sender_count;
    cin.ignore();

    try {
        Receiver receiver(filename, message_count, sender_count);
        vector<PROCESS_INFORMATION> processes(sender_count);

        string mutexName = filename + "_mutex";
        string emptySemName = filename + "_empty";
        string fullSemName = filename + "_full";

        for (int i = 0; i < sender_count; i++) {
            string eventName = filename + "_ready_" + to_string(i + 1);
            string command = "sender.exe \"" + filename + "\" \"" +
                eventName + "\" \"" + mutexName + "\" \"" +
                emptySemName + "\" \"" + fullSemName + "\" \"" +
                to_string(message_count) + "\"";

            if (!launchSenderProcess(command, processes[i])) {
                SetColor(RED);
                cerr << "Failed to start sender " << (i + 1) << endl;
                SetColor(WHITE);
                return ERROR_EXIT;
            }
            SetColor(GREEN);
            cout << "Sender " << (i + 1) << " started" << endl;
            SetColor(WHITE);
        }
        receiver.waitForAllSenders();

        receiver.run();

        for (int i = 0; i < sender_count; i++) {
            if (processes[i].hThread) {
                PostThreadMessage(processes[i].dwThreadId, WM_QUIT, 0, 0);
            }

            WaitForSingleObject(processes[i].hProcess, 5000);
            CloseHandle(processes[i].hProcess);
            CloseHandle(processes[i].hThread);
        }

        SetColor(GREEN);
        cout << "Receiver finished" << endl;
        SetColor(WHITE);

    }
    catch (const exception& e) {
        SetColor(RED);
        cerr << "Error: " << e.what() << endl;
        SetColor(WHITE);
        return ERROR_EXIT; 
    }

    return SUCCESS_EXIT; 
}
