#include "client.h"
#include <iostream>
#include <cstring>
#include <string>
#include <limits>
using namespace std;

Client::Client(int id) : client_id(id), hPipe(INVALID_HANDLE_VALUE), connected(false) {
    pipeName = "\\\\.\\pipe\\lab5_pipe_" + to_string(client_id);
}

Client::~Client() {
    disconnectFromServer();
}

bool Client::connectToServer() {
    if (connected && hPipe != INVALID_HANDLE_VALUE) {
        return true;
    }

    for (int attempt = 0; attempt < 10; attempt++) {
        hPipe = CreateFileA(
            pipeName.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
        );

        if (hPipe != INVALID_HANDLE_VALUE) {
            DWORD mode = PIPE_READMODE_MESSAGE;
            if (SetNamedPipeHandleState(hPipe, &mode, NULL, NULL)) {
                connected = true;
                cout << "Connected to server successfully." << endl;
                return true;
            }
            CloseHandle(hPipe);
            hPipe = INVALID_HANDLE_VALUE;
        }

        DWORD error = GetLastError();
        if (error == ERROR_PIPE_BUSY) {
            cout << "Pipe is busy, waiting..." << endl;
            if (!WaitNamedPipeA(pipeName.c_str(), 30000)) {
                cerr << "WaitNamedPipe timeout" << endl;
                continue;
            }
        }
        else if (error == ERROR_FILE_NOT_FOUND) {
            cout << "Pipe not found, waiting for server..." << endl;
            Sleep(1000);
        }
        else {
            cerr << "CreateFile error: " << error << endl;
            break;
        }
    }

    cerr << "Failed to connect to server after multiple attempts" << endl;
    return false;
}

void Client::disconnectFromServer() {
    if (connected && hPipe != INVALID_HANDLE_VALUE) {
        try {
            Request request;
            request.operationType = OperationType::END;
            request.employeeId = 0;

            DWORD bytesWritten;
            WriteFile(hPipe, &request, sizeof(Request), &bytesWritten, NULL);
        }
        catch (...) {
        }

        if (hPipe != INVALID_HANDLE_VALUE && hPipe != NULL) {
            CloseHandle(hPipe);
        }
        hPipe = INVALID_HANDLE_VALUE;
        connected = false;
        cout << "Disconnected from server." << endl;
    }
}

bool Client::sendRequest(const Request& request) {
    if (!connected || hPipe == INVALID_HANDLE_VALUE) {
        cerr << "Not connected to server" << endl;
        return false;
    }

    DWORD bytesWritten;
    if (!WriteFile(hPipe, &request, sizeof(Request), &bytesWritten, NULL)) {
        cerr << "WriteFile failed: " << GetLastError() << endl;
        connected = false;
        return false;
    }
    return bytesWritten == sizeof(Request);
}

bool Client::receiveResponse(Response& response) {
    if (!connected || hPipe == INVALID_HANDLE_VALUE) {
        cerr << "Not connected to server" << endl;
        return false;
    }

    DWORD bytesRead;
    if (!ReadFile(hPipe, &response, sizeof(Response), &bytesRead, NULL)) {
        DWORD error = GetLastError();
        if (error != ERROR_BROKEN_PIPE) {
            cerr << "ReadFile failed: " << error << endl;
        }
        connected = false;
        return false;
    }
    return bytesRead == sizeof(Response);
}

int getValidIntInput() {
    int value;
    while (!(cin >> value)) {
        cout << "Invalid input. Please enter a number: ";
        cin.clear();
        cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
    }
    return value;
}

double getValidDoubleInput() {
    double value;
    while (!(cin >> value)) {
        cout << "Invalid input. Please enter a number: ";
        cin.clear();
        cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
    }
    return value;
}

string getValidStringInput() {
    string value;
    cin >> value;
    return value;
}

void Client::modifyRecord() {
    cout << "Enter employee ID to modify: ";
    int employeeId = getValidIntInput();

    Request lock_request;
    lock_request.operationType = OperationType::WRITE;
    lock_request.employeeId = employeeId;
    lock_request.emp = Employee{ 0, "", 0.0 };

    if (!sendRequest(lock_request)) {
        cerr << "Failed to send lock request" << endl;
        return;
    }

    Response lock_response;
    if (!receiveResponse(lock_response)) {
        cerr << "Failed to receive lock response" << endl;
        return;
    }

    if (!lock_response.success) {
        cout << "Record " << employeeId << " is locked by another client!" << endl;
        return;
    }

    cout << "\nCurrent employee data:" << endl;
    cout << lock_response.emp;

    Employee emp = lock_response.emp;

    cout << "\nEnter new data (enter - to keep current):" << endl;

    cout << "New name (max 9 chars): ";
    string name = getValidStringInput();
    if (name != "-") {
        strncpy(emp.name, name.c_str(), sizeof(emp.name) - 1);
        emp.name[sizeof(emp.name) - 1] = '\0';
    }

    cout << "New work hours: ";
    string hours_str = getValidStringInput();
    if (hours_str != "-") {
        try {
            emp.hours = stod(hours_str);
        }
        catch (...) {
            cout << "Invalid number format. Keeping old value." << endl;
        }
    }

    cout << "\nEnter 'send' to send modified record to server (or 'cancel' to abort): ";
    string command = getValidStringInput();

    if (command == "send") {
        Request write_request;
        write_request.operationType = OperationType::WRITE;
        write_request.employeeId = employeeId;
        write_request.emp = emp;

        if (!sendRequest(write_request)) {
            cerr << "Failed to send write request" << endl;
            return;
        }

        Response write_response;
        if (!receiveResponse(write_response)) {
            cerr << "Failed to receive write response" << endl;
            return;
        }

        if (write_response.success) {
            cout << "Record updated successfully!" << endl;
        }
        else {
            cout << "Failed to update record!" << endl;
        }
    }
    else {
        cout << "Modification cancelled." << endl;
    }

    cout << "\nEnter 'finish' to release lock: ";
    command = getValidStringInput();

    if (command == "finish") {
        Request end_request;
        end_request.operationType = OperationType::END;
        end_request.employeeId = employeeId;

        if (!sendRequest(end_request)) {
            cerr << "Failed to send finish request" << endl;
        }
        else {
            Response end_response;
            if (receiveResponse(end_response)) {
                cout << "Lock released successfully." << endl;
            }
        }
    }
}

void Client::readRecord() {
    cout << "Enter employee ID to read: ";
    int employeeId = getValidIntInput();

    Request read_request;
    read_request.operationType = OperationType::READ;
    read_request.employeeId = employeeId;

    if (!sendRequest(read_request)) {
        cerr << "Failed to send read request" << endl;
        return;
    }

    Response read_response;
    if (!receiveResponse(read_response)) {
        cerr << "Failed to receive response" << endl;
        return;
    }

    if (!read_response.success) {
        cout << "Employee with ID " << employeeId << " not found or locked for writing!" << endl;
    }
    else {
        cout << "\nEmployee data:" << endl;
        cout << read_response.emp;
    }

    cout << "\nEnter 'finish' to release lock: ";
    string command = getValidStringInput();

    if (command == "finish") {
        Request end_request;
        end_request.operationType = OperationType::END;
        end_request.employeeId = employeeId;

        if (!sendRequest(end_request)) {
            cerr << "Failed to send finish request" << endl;
        }
        else {
            Response end_response;
            if (receiveResponse(end_response)) {
                cout << "Lock released successfully." << endl;
            }
        }
    }
}

void Client::run() {
    cout << "=== Client " << client_id << " started ===" << endl;

    if (!connectToServer()) {
        cerr << "Failed to connect to server. Exiting." << endl;
        return;
    }

    try {
        while (true) {
            cout << "\n=== Client " << client_id << " Menu ===" << endl;
            cout << "1. Modify record" << endl;
            cout << "2. Read record" << endl;
            cout << "3. Exit" << endl;
            cout << "Your choice: ";

            int choice = getValidIntInput();

            switch (choice) {
            case 1:
                modifyRecord();
                break;
            case 2:
                readRecord();
                break;
            case 3:
                cout << "Client " << client_id << " stopping..." << endl;
                disconnectFromServer();
                return;
            default:
                cout << "Invalid choice! Please enter 1, 2, or 3." << endl;
            }
        }
    }
    catch (const exception& e) {
        cerr << "Client error: " << e.what() << endl;
        disconnectFromServer();
    }
    catch (...) {
        cerr << "Unknown client error" << endl;
        disconnectFromServer();
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: client.exe <client_id>" << endl;
        return 1;
    }

    try {
        int client_id = stoi(argv[1]);
        Client client(client_id);
        client.run();
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}