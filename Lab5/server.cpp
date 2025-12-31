#include "server.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <cstring>
#include <algorithm>
#include <vector>
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
    cout << "Input name of binary file: ";
    cin >> filename;

    int employeesNumber;
    cout << "Input number of employees: ";
    cin >> employeesNumber;

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
        cerr << "File creation error!" << endl;
        exit(1);
    }

    for (int i = 0; i < employeesNumber; i++) {
        Employee emp;
        cout << "\nEmployee #" << i + 1 << ":" << endl;
        cin >> emp;

        employees.push_back(emp);
        file.write(reinterpret_cast<const char*>(&emp), sizeof(Employee));

        lock_guard<mutex> lock(map_mutex);
        record_locks[emp.id] = new RecordLock();
    }

    file.close();
}

void Server::printFile() {
    cout << "\nYour file:" << endl;
    ifstream file(filename, ios::binary);
    if (!file) {
        cerr << "File open error!" << endl;
        return;
    }

    Employee emp;
    int i = 1;
    while (file.read(reinterpret_cast<char*>(&emp), sizeof(Employee))) {
        cout << "Employee #" << i++ << ":" << endl;
        cout << emp << endl;
    }
    file.close();
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
        cerr << "File update error!" << endl;
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
        return false;
    }

    read_locks[employeeId].insert(client_id);
    cout << "Client " << client_id << " locked record " << employeeId << " for reading" << endl;
    return true;
}

bool Server::lockForWrite(int client_id, int employeeId) {
    lock_guard<mutex> lock(lock_mutex);

    auto read_it = read_locks.find(employeeId);
    auto write_it = write_locks.find(employeeId);

    if ((read_it != read_locks.end() && !read_it->second.empty()) ||
        write_it != write_locks.end()) {
        return false;
    }

    write_locks[employeeId] = client_id;
    cout << "Client " << client_id << " locked record " << employeeId << " for writing" << endl;
    return true;
}

bool Server::unlockRecord(int client_id, int employeeId) {
    lock_guard<mutex> lock(lock_mutex);

    auto write_it = write_locks.find(employeeId);
    if (write_it != write_locks.end() && write_it->second == client_id) {
        write_locks.erase(write_it);
        cout << "Client " << client_id << " unlocked record " << employeeId << " (write)" << endl;
        return true;
    }

    auto read_it = read_locks.find(employeeId);
    if (read_it != read_locks.end()) {
        auto& clients = read_it->second;
        auto client_it = clients.find(client_id);
        if (client_it != clients.end()) {
            clients.erase(client_it);
            cout << "Client " << client_id << " unlocked record " << employeeId << " (read)" << endl;

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
        return false;
    }

    RecordLock* lock = nullptr;
    {
        lock_guard<mutex> guard(map_mutex);
        auto it = record_locks.find(employeeId);
        if (it == record_locks.end()) {
            return false;
        }
        lock = it->second;
    }

    lock->lock_write();

    bool success = false;
    if (findEmployeeIndex(employeeId) >= 0) {
        updateEmployee(emp);
        success = true;
    }

    lock->unlock_write();
    return success;
}

void Server::handleClient(int client_id, HANDLE hPipe) {
    cout << "Client " << client_id << " connected" << endl;

    DWORD bytesRead, bytesWritten;
    int current_locked_record = -1;
    bool is_write_lock = false;

    while (running) {
        Request request;
        if (!ReadFile(hPipe, &request, sizeof(Request), &bytesRead, NULL)) {
            DWORD error = GetLastError();
            if (error == ERROR_BROKEN_PIPE || error == ERROR_NO_DATA) {
                cout << "Client " << client_id << " disconnected" << endl;
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
            }
            else {
                response.success = false;
                cerr << "Client " << client_id << " cannot read record "
                    << request.employeeId << " - locked by another client for writing" << endl;
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
                }
                else {
                    response.success = false;
                    cerr << "Client " << client_id << " cannot write record "
                        << request.employeeId << " - locked by another client" << endl;
                }
                WriteFile(hPipe, &response, sizeof(Response), &bytesWritten, NULL);
            }
            else {
                if (current_locked_record == request.employeeId && is_write_lock) {
                    response.success = writeRecord(client_id, request.employeeId, request.emp);
                    WriteFile(hPipe, &response, sizeof(Response), &bytesWritten, NULL);
                }
                else {
                    response.success = false;
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
            cout << "Client " << client_id << " released all locks" << endl;

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
    vector<thread> threads;
    vector<HANDLE> pipes;

    for (int i = 0; i < client_count; i++) {
        string pipeName = PIPE_NAME_BASE + to_string(i);

        HANDLE hPipe = CreateNamedPipeA(
            pipeName.c_str(),
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            4096,
            4096,
            0,
            NULL
        );

        if (hPipe == INVALID_HANDLE_VALUE) {
            cerr << "Failed to create pipe for client " << i << endl;
            continue;
        }

        pipes.push_back(hPipe);
        cout << "Created pipe: " << pipeName << endl;
    }

    for (int i = 0; i < (int)pipes.size(); i++) {
        threads.emplace_back([this, i, hPipe = pipes[i]]() {
            cout << "Waiting for client " << i << "..." << endl;

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

        cout << "Starting client " << i << "..." << endl;

        if (CreateProcessA(NULL, cmdLine, NULL, NULL, FALSE,
            CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
            processes.push_back(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        else {
            cerr << "Failed to create client " << i << endl;
        }
    }

    for (HANDLE hProcess : processes) {
        WaitForSingleObject(hProcess, INFINITE);
        CloseHandle(hProcess);
        cout << "Client process terminated" << endl;
    }

    Sleep(2000);

    running = false;

    cout << "Server stopping..." << endl;


    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    cout << "All clients finished and resources cleaned up" << endl;
}

void Server::run() {
    cout << "=== Server Started ===" << endl;

    createFile();
    printFile();

    int processesCount;
    cout << "\nInput count of processes Client: ";
    cin >> processesCount;

    startClients(processesCount);

    cout << "\nYour file after all modifications:" << endl;
    printFile();

    cout << "Input any key to finish program: ";
    char c;
    cin >> c;
}