#include "client.h"
#include <iostream>
#include <cstring>
#include <limits>
using namespace std;

Client::Client(int id) : client_id(id), hPipe(INVALID_HANDLE_VALUE), connected(false) {
    pipeName = CreatePipeName(client_id);
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
                PrintSuccess("Connected to server successfully as Client " + to_string(client_id));
                return true;
            }
            CloseHandle(hPipe);
            hPipe = INVALID_HANDLE_VALUE;
        }

        DWORD error = GetLastError();
        if (error == ERROR_PIPE_BUSY) {
            PrintWarning("Pipe is busy, waiting...");
            if (!WaitNamedPipeA(pipeName.c_str(), PIPE_TIMEOUT)) {
                PrintError("WaitNamedPipe timeout");
                continue;
            }
        }
        else if (error == ERROR_FILE_NOT_FOUND) {
            PrintWarning("Pipe not found, waiting for server...");
            Sleep(WAIT_RECONNECT_DELAY);
        }
        else {
            PrintError("CreateFile error: " + to_string(error));
            break;
        }
    }

    PrintError("Failed to connect to server after multiple attempts");
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
        PrintInfo("Disconnected from server");
    }
}

bool Client::sendRequest(const Request& request) {
    if (!connected || hPipe == INVALID_HANDLE_VALUE) {
        PrintError("Not connected to server");
        return false;
    }

    DWORD bytesWritten;
    if (!WriteFile(hPipe, &request, sizeof(Request), &bytesWritten, NULL)) {
        PrintError("WriteFile failed: " + to_string(GetLastError()));
        connected = false;
        return false;
    }
    return bytesWritten == sizeof(Request);
}

bool Client::receiveResponse(Response& response) {
    if (!connected || hPipe == INVALID_HANDLE_VALUE) {
        PrintError("Not connected to server");
        return false;
    }

    DWORD bytesRead;
    if (!ReadFile(hPipe, &response, sizeof(Response), &bytesRead, NULL)) {
        DWORD error = GetLastError();
        if (error != ERROR_BROKEN_PIPE) {
            PrintError("ReadFile failed: " + to_string(error));
        }
        connected = false;
        return false;
    }
    return bytesRead == sizeof(Response);
}

void Client::modifyRecord() {
    PrintSeparator('-');
    SetColor(CYAN);
    cout << "=== MODIFY RECORD ===" << endl;
    SetColor(WHITE);

    cout << "Enter employee ID to modify: ";
    int employeeId = GetValidIntInput();

    if (!IsValidEmployeeId(employeeId)) {
        PrintError("Invalid employee ID! Must be between 1 and " + to_string(MAX_EMPLOYEES));
        return;
    }


    Request lock_request;
    lock_request.operationType = OperationType::WRITE;
    lock_request.employeeId = employeeId;
    lock_request.emp = Employee{ 0, "", 0.0 };

    if (!sendRequest(lock_request)) {
        PrintError("Failed to send lock request");
        return;
    }

    Response lock_response;
    if (!receiveResponse(lock_response)) {
        PrintError("Failed to receive lock response");
        return;
    }

    if (!lock_response.success) {
        PrintWarning("Record " + to_string(employeeId) +
            " is locked by another client or doesn't exist!");
        return;
    }


    PrintSeparator('-');
    SetColor(YELLOW);
    cout << "CURRENT EMPLOYEE DATA:" << endl;
    SetColor(WHITE);
    cout << lock_response.emp;

    Employee emp = lock_response.emp;

    PrintSeparator('-');
    SetColor(GREEN);
    cout << "ENTER NEW DATA (enter - to keep current):" << endl;
    SetColor(WHITE);


    cout << "New name (max 9 chars, current: \"" << emp.name << "\"): ";
    string name = GetValidStringInput();
    if (name != "-") {
        if (name.length() > 0 && name.length() <= MAX_NAME_LENGTH) {
            strncpy(emp.name, name.c_str(), sizeof(emp.name) - 1);
            emp.name[sizeof(emp.name) - 1] = '\0';
        }
        else if (name.length() > MAX_NAME_LENGTH) {
            PrintWarning("Name too long! Truncating to " + to_string(MAX_NAME_LENGTH) + " characters.");
            strncpy(emp.name, name.c_str(), sizeof(emp.name) - 1);
            emp.name[sizeof(emp.name) - 1] = '\0';
        }
    }


    cout << "New work hours (current: " << emp.hours << "): ";
    string hours_str = GetValidStringInput();
    if (hours_str != "-") {
        try {
            double new_hours = stod(hours_str);
            if (new_hours >= 0) {
                emp.hours = new_hours;
            }
            else {
                PrintWarning("Work hours cannot be negative! Keeping current value.");
            }
        }
        catch (...) {
            PrintWarning("Invalid number format. Keeping current value.");
        }
    }


    PrintSeparator('-');
    cout << "Enter 'send' to send modified record to server (or 'cancel' to abort): ";
    string command = GetValidStringInput();

    if (command == "send") {
        Request write_request;
        write_request.operationType = OperationType::WRITE;
        write_request.employeeId = employeeId;
        write_request.emp = emp;

        if (!sendRequest(write_request)) {
            PrintError("Failed to send write request");
            return;
        }

        Response write_response;
        if (!receiveResponse(write_response)) {
            PrintError("Failed to receive write response");
            return;
        }

        if (write_response.success) {
            PrintSuccess("Record updated successfully!");
        }
        else {
            PrintError("Failed to update record!");
        }
    }
    else {
        PrintWarning("Modification cancelled.");
    }

    bool lock_released = false;
    do {
        PrintSeparator('-');
        cout << "Enter 'finish' to release lock (this is required): ";
        command = GetValidStringInput();

        if (command == "finish") {
            Request end_request;
            end_request.operationType = OperationType::END;
            end_request.employeeId = employeeId;

            if (!sendRequest(end_request)) {
                PrintError("Failed to send finish request");
            }
            else {
                Response end_response;
                if (receiveResponse(end_response)) {
                    PrintInfo("Lock released successfully.");
                    lock_released = true;
                }
            }
        }
        else {
            PrintError("Invalid input! You must enter 'finish' to release the lock.");
            PrintWarning("Record " + to_string(employeeId) + " remains locked.");
            PrintWarning("Other clients cannot access this record until you release it!");
        }
    } while (!lock_released);
}

void Client::readRecord() {
    PrintSeparator('-');
    SetColor(CYAN);
    cout << "=== READ RECORD ===" << endl;
    SetColor(WHITE);

    cout << "Enter employee ID to read: ";
    int employeeId = GetValidIntInput();

    if (!IsValidEmployeeId(employeeId)) {
        PrintError("Invalid employee ID!");
        return;
    }

    Request read_request;
    read_request.operationType = OperationType::READ;
    read_request.employeeId = employeeId;

    if (!sendRequest(read_request)) {
        PrintError("Failed to send read request");
        return;
    }

    Response read_response;
    if (!receiveResponse(read_response)) {
        PrintError("Failed to receive response");
        return;
    }

    if (!read_response.success) {
        PrintWarning("Employee with ID " + to_string(employeeId) + " not found or locked for writing!");
    }
    else {
        PrintSeparator('-');
        SetColor(YELLOW);
        cout << "EMPLOYEE DATA:" << endl;
        SetColor(WHITE);
        cout << read_response.emp;
    }

    bool lock_released = false;
    do {
        PrintSeparator('-');
        cout << "Enter 'finish' to release read lock (this is required): ";
        string command = GetValidStringInput();

        if (command == "finish") {
            Request end_request;
            end_request.operationType = OperationType::END;
            end_request.employeeId = employeeId;

            if (!sendRequest(end_request)) {
                PrintError("Failed to send finish request");
            }
            else {
                Response end_response;
                if (receiveResponse(end_response)) {
                    PrintInfo("Read lock released successfully.");
                    lock_released = true;
                }
            }
        }
        else {
            PrintError("Invalid input! You must enter 'finish' to release the read lock.");
            PrintWarning("Record " + to_string(employeeId) + " remains locked for reading.");
            PrintWarning("Other clients cannot write to this record until you release it!");
        }
    } while (!lock_released);
}

void Client::run() {
    SetColor(BRIGHT_MAGENTA);
    PrintSeparator('=', 40);
    cout << "=== CLIENT " << client_id << " STARTED ===" << endl;
    PrintSeparator('=', 40);
    SetColor(WHITE);

    if (!connectToServer()) {
        PrintError("Failed to connect to server. Exiting.");
        return;
    }

    try {
        while (true) {
            PrintSeparator('=');
            SetColor(BRIGHT_CYAN);
            cout << "=== CLIENT " << client_id << " MENU ===" << endl;
            SetColor(WHITE);
            cout << "1. Modify record" << endl;
            cout << "2. Read record" << endl;
            cout << "3. Exit" << endl;
            cout << "Your choice: ";

            int choice = GetValidIntInput();

            switch (choice) {
            case 1:
                modifyRecord();
                break;
            case 2:
                readRecord();
                break;
            case 3:
                PrintSeparator('=');
                SetColor(YELLOW);
                cout << "Client " << client_id << " stopping..." << endl;
                SetColor(WHITE);
                disconnectFromServer();
                return;
            default:
                PrintError("Invalid choice! Please enter 1, 2, or 3.");
            }
        }
    }
    catch (const exception& e) {
        PrintError("Client error: " + string(e.what()));
        disconnectFromServer();
    }
    catch (...) {
        PrintError("Unknown client error");
        disconnectFromServer();
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintError("Usage: client.exe <client_id>");
        return APP_ERROR_EXIT;
    }

    try {
        int client_id = stoi(argv[1]);

        if (client_id < 0 || client_id >= MAX_PIPE_INSTANCES) {
            PrintError("Invalid client ID! Must be between 0 and " + to_string(MAX_PIPE_INSTANCES - 1));
            return ERROR_INVALID_DATA;
        }

        Client client(client_id);
        client.run();
    }
    catch (const exception& e) {
        PrintError("Error: " + string(e.what()));
        return APP_ERROR_EXIT;
    }

    return APP_SUCCESS_EXIT; 
}
