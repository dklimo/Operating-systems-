#include "server.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <cstring>
#include <algorithm>
using namespace std;

Server::Server() : running(true) {}

Server::~Server() {
    lock_guard<mutex> lock(map_mutex);
    for (auto& pair : record_locks) {
        delete pair.second;
    }
    record_locks.clear();
}

void Server::createFile() {
    PrintColored("\n=== CREATE BINARY FILE ===\n", CYAN);

    cout << "Input name of binary file: ";
    cin >> filename;

    if (FileExists(filename)) {
        PrintWarning("File already exists. It will be overwritten.");
    }

    int employeesNumber;
    cout << "Input number of employees (max " << MAX_EMPLOYEES << "): ";
    employeesNumber = GetValidIntInput();

    if (employeesNumber <= 0 || employeesNumber > MAX_EMPLOYEES) {
        PrintError("Invalid number of employees!");
        exit(APP_ERROR_INVALID_DATA);
    }

    employees.clear();

    {
        lock_guard<mutex> lock(map_mutex);
        for (auto& pair : record_locks) {
            delete pair.second;
        }
        record_locks.clear();
    }

    ofstream file(filename, ios::binary);
    if (!file) {
        PrintError("File creation error!");
        exit(APP_ERROR_FILE_NOT_FOUND);
    }

    for (int i = 0; i < employeesNumber; i++) {
        Employee emp;
        PrintSeparator('-');
        cout << "Employee #" << i + 1 << ":" << endl;
        cin >> emp;
        bool id_exists = false;
        for (const auto& existing : employees) {
            if (existing.id == emp.id) {
                PrintError("Employee ID " + to_string(emp.id) + " already exists!");
                id_exists = true;
                break;
            }
        }
        if (id_exists) {
            i--;
            continue;
        }

        employees.push_back(emp);
        file.write(reinterpret_cast<const char*>(&emp), sizeof(Employee));

        lock_guard<mutex> lock(map_mutex);
        record_locks[emp.id] = new RecordLock();
    }

    file.close();
    PrintSuccess("File created successfully with " + to_string(employeesNumber) + " employees");
}

void Server::printFile() {
    PrintSeparator('=');
    PrintColored("FILE CONTENTS:\n", YELLOW);

    ifstream file(filename, ios::binary);
    if (!file) {
        PrintError("File open error!");
        return;
    }

    Employee emp;
    int i = 1;
    while (file.read(reinterpret_cast<char*>(&emp), sizeof(Employee))) {
        PrintSeparator('-');
        cout << "Employee #" << i++ << ":" << endl;
        cout << emp << endl;
    }
    file.close();

    PrintSeparator('=');
}

int Server::findEmployeeIndex(int id) {
    for (size_t i = 0; i < employees.size(); i++) {
        if (employees[i].id == id) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void Server::updateEmployee(const Employee& updated) {
    for (auto& e : employees) {
        if (e.id == updated.id) {
            e = updated;
            break;
        }
    }

    fstream file(filename, ios::binary | ios::in | ios::out);
    if (!file) {
        PrintError("File update error!");
        return;
    }

    Employee temp;
    while (file.read(reinterpret_cast<char*>(&temp), sizeof(Employee))) {
        if (temp.id == updated.id) {
            file.seekp(-static_cast<streamoff>(sizeof(Employee)), ios::cur);
            file.write(reinterpret_cast<const char*>(&updated), sizeof(Employee));
            break;
        }
    }
    file.close();
}

bool Server::lockForRead(int client_id, int employeeId) {
    lock_guard<mutex> lock(lock_mutex);

    auto write_it = write_locks.find(employeeId);
    if (write_it != write_locks.end()) {
        PrintInfo("Client " + to_string(client_id) + " cannot read record " +
            to_string(employeeId) + " - locked for writing by client " +
            to_string(write_it->second));
        return false;
    }

    read_locks[employeeId].insert(client_id);
    PrintInfo("Client " + to_string(client_id) + " locked record " +
        to_string(employeeId) + " for reading");
    return true;
}

bool Server::lockForWrite(int client_id, int employeeId) {
    lock_guard<mutex> lock(lock_mutex);

    auto read_it = read_locks.find(employeeId);
    auto write_it = write_locks.find(employeeId);

    if ((read_it != read_locks.end() && !read_it->second.empty()) ||
        write_it != write_locks.end()) {
        string msg = "Client " + to_string(client_id) + " cannot write record " +
            to_string(employeeId) + " - locked by ";

        if (write_it != write_locks.end()) {
            msg += "client " + to_string(write_it->second) + " for writing";
        }
        else if (read_it != read_locks.end() && !read_it->second.empty()) {
            msg += to_string(read_it->second.size()) + " clients for reading";
        }

        PrintInfo(msg);
        return false;
    }

    write_locks[employeeId] = client_id;
    PrintInfo("Client " + to_string(client_id) + " locked record " +
        to_string(employeeId) + " for writing");
    return true;
}

bool Server::unlockRecord(int client_id, int employeeId) {
    lock_guard<mutex> lock(lock_mutex);

    auto write_it = write_locks.find(employeeId);
    if (write_it != write_locks.end() && write_it->second == client_id) {
        write_locks.erase(write_it);
        PrintInfo("Client " + to_string(client_id) + " unlocked record " +
            to_string(employeeId) + " (write)");
        return true;
    }

    auto read_it = read_locks.find(employeeId);
    if (read_it != read_locks.end()) {
        auto& clients = read_it->second;
        auto client_it = clients.find(client_id);
        if (client_it != clients.end()) {
            clients.erase(client_it);
            PrintInfo("Client " + to_string(client_id) + " unlocked record " +
                to_string(employeeId) + " (read)");

            if (clients.empty()) {
                read_locks.erase(read_it);
            }
            return true;
        }
    }

    return false;
}

Employee Server::readRecord(int client_id, int employeeId) {
    RecordLock* lock = nullptr;
    {
        lock_guard<mutex> guard(map_mutex);
        auto it = record_locks.find(employeeId);
        if (it == record_locks.end()) {
            return Employee{ 0, "", 0.0 };
        }
        lock = it->second;
    }

    lock->lock_read();

    Employee emp{ 0, "", 0.0 };
    int index = findEmployeeIndex(employeeId);
    if (index >= 0) {
        emp = employees[index];
    }

    lock->unlock_read();
    return emp;
}

bool Server::writeRecord(int client_id, int employeeId, const Employee& emp) {
    if (emp.id != employeeId) {
        PrintWarning("Client " + to_string(client_id) +
            " sent mismatched employee ID: " + to_string(emp.id) +
            " != " + to_string(employeeId));
        return false;
    }

    RecordLock* lock = nullptr;
    {
        lock_guard<mutex> guard(map_mutex);
        auto it = record_locks.find(employeeId);
        if (it == record_locks.end()) {
            PrintWarning("Client " + to_string(client_id) +
                " tried to write non-existent record: " +
                to_string(employeeId));
            return false;
        }
        lock = it->second;
    }

    lock->lock_write();

    bool success = false;
    int index = findEmployeeIndex(employeeId);
    if (index >= 0) {
        updateEmployee(emp);
        success = true;
        PrintInfo("Client " + to_string(client_id) +
            " successfully updated record " + to_string(employeeId));
    }
    else {
        PrintWarning("Client " + to_string(client_id) +
            " tried to write non-existent employee ID: " +
            to_string(employeeId));
    }

    lock->unlock_write();
    return success;
}

void Server::handleClient(int client_id, HANDLE hPipe) {
    SetColor(BRIGHT_GREEN);
    cout << "Client " << client_id << " connected" << endl;
    SetColor(WHITE);

    DWORD bytesRead, bytesWritten;
    int current_locked_record = -1;
    bool is_write_lock = false;

    while (running) {
        Request request;
        if (!ReadFile(hPipe, &request, sizeof(Request), &bytesRead, NULL)) {
            DWORD error = GetLastError();
            if (error == ERROR_BROKEN_PIPE || error == ERROR_NO_DATA) {
                SetColor(YELLOW);
                cout << "Client " << client_id << " disconnected" << endl;
                SetColor(WHITE);
                break;
            }
            continue;
        }

        if (!running) break;

        Response response{};

        switch (request.operationType) {
        case OperationType::READ: {
            if (lockForRead(client_id, request.employeeId)) {
                current_locked_record = request.employeeId;
                is_write_lock = false;

                response.emp = readRecord(client_id, request.employeeId);
                response.success = (response.emp.id != 0);

                if (!response.success) {
                    PrintWarning("Client " + to_string(client_id) +
                        " requested non-existent employee ID: " +
                        to_string(request.employeeId));
                }
            }
            else {
                response.success = false;
            }
            WriteFile(hPipe, &response, sizeof(Response), &bytesWritten, NULL);
            break;
        }

        case OperationType::WRITE: {
            if (request.emp.id == 0) {
                if (lockForWrite(client_id, request.employeeId)) {
                    current_locked_record = request.employeeId;
                    is_write_lock = true;

                    response.emp = readRecord(client_id, request.employeeId);
                    response.success = (response.emp.id != 0);

                    if (!response.success) {
                        PrintWarning("Client " + to_string(client_id) +
                            " requested to write non-existent employee ID: " +
                            to_string(request.employeeId));
                    }
                }
                else {
                    response.success = false;
                }
                WriteFile(hPipe, &response, sizeof(Response), &bytesWritten, NULL);
            }
            else {
                if (current_locked_record == request.employeeId && is_write_lock) {
                    response.success = writeRecord(client_id, request.employeeId, request.emp);
                    if (response.success) {
                        PrintSuccess("Client " + to_string(client_id) +
                            " updated record " + to_string(request.employeeId));
                    }
                    WriteFile(hPipe, &response, sizeof(Response), &bytesWritten, NULL);
                }
                else {
                    response.success = false;
                    PrintWarning("Client " + to_string(client_id) +
                        " tried to write without proper lock on record " +
                        to_string(request.employeeId));
                    WriteFile(hPipe, &response, sizeof(Response), &bytesWritten, NULL);
                }
            }
            break;
        }

        case OperationType::END:
            if (current_locked_record != -1) {
                unlockRecord(client_id, current_locked_record);
                current_locked_record = -1;
            }

            response.success = true;
            WriteFile(hPipe, &response, sizeof(Response), &bytesWritten, NULL);

            PrintInfo("Client " + to_string(client_id) + " released all locks");
            break;
        }
    }

    if (current_locked_record != -1) {
        unlockRecord(client_id, current_locked_record);
    }

    if (hPipe != INVALID_HANDLE_VALUE && hPipe != NULL) {
        FlushFileBuffers(hPipe);
        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);
    }
}

void Server::startClients(int client_count) {
    PrintSeparator('=');
    PrintColored("STARTING CLIENTS\n", CYAN);

    vector<thread> threads;
    vector<HANDLE> pipes;

    for (int i = 0; i < client_count; i++) {
        string pipeName = CreatePipeName(i);

        HANDLE hPipe = CreateNamedPipeA(
            pipeName.c_str(),
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            MAX_PIPE_INSTANCES,
            PIPE_BUFFER_SIZE,
            PIPE_BUFFER_SIZE,
            PIPE_TIMEOUT,
            NULL
        );

        if (hPipe == INVALID_HANDLE_VALUE) {
            PrintError("Failed to create pipe for client " + to_string(i));
            continue;
        }

        pipes.push_back(hPipe);
        PrintInfo("Created pipe: " + pipeName);
    }

    Sleep(WAIT_SERVER_STARTUP_DELAY);

    for (int i = 0; i < (int)pipes.size(); i++) {
        threads.emplace_back([this, i, hPipe = pipes[i]]() {
            PrintInfo("Waiting for client " + to_string(i) + "...");

            if (ConnectNamedPipe(hPipe, NULL) || GetLastError() == ERROR_PIPE_CONNECTED) {
                this->handleClient(i, hPipe);
            }
            else {
                if (hPipe != INVALID_HANDLE_VALUE && hPipe != NULL) {
                    CloseHandle(hPipe);
                }
            }
            });
    }

    vector<HANDLE> processes;
    for (int i = 0; i < client_count; i++) {
        STARTUPINFOA si{};
        PROCESS_INFORMATION pi{};
        si.cb = sizeof(si);

        string cmd = "client.exe " + to_string(i);
        char cmdLine[256];
        strcpy_s(cmdLine, cmd.c_str());

        PrintInfo("Starting client " + to_string(i) + "...");

        if (CreateProcessA(NULL, cmdLine, NULL, NULL, FALSE,
            CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
            processes.push_back(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        else {
            PrintError("Failed to create client " + to_string(i));
        }
    }

    for (HANDLE hProcess : processes) {
        WaitForSingleObject(hProcess, INFINITE);
        CloseHandle(hProcess);
        PrintInfo("Client process terminated");
    }

    Sleep(WAIT_SERVER_STARTUP_DELAY);

    running = false;

    PrintSeparator('=');
    PrintColored("SERVER STOPPING...\n", YELLOW);

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    PrintSuccess("All clients finished and resources cleaned up");
    PrintSeparator('=');
}

void Server::run() {
    SetColor(BRIGHT_CYAN);
    PrintSeparator('=', 60);
    cout << "=== SERVER STARTED ===" << endl;
    PrintSeparator('=', 60);
    SetColor(WHITE);

    createFile();
    printFile();

    int processesCount;
    cout << "\nInput count of client processes (max " << MAX_PIPE_INSTANCES << "): ";
    processesCount = GetValidIntInput();

    if (processesCount <= 0 || processesCount > MAX_PIPE_INSTANCES) {
        PrintError("Invalid number of clients!");
        return;
    }

    startClients(processesCount);

    PrintSeparator('=');
    PrintColored("FINAL FILE STATE:\n", YELLOW);
    printFile();

    cout << "\n";
    SetColor(CYAN);
    cout << "Input any key to finish program: ";
    SetColor(WHITE);
    char c;
    cin >> c;
}
