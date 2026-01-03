#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <atomic>
#include <set>
#include <windows.h>
#include "employee.h"
#include "record_lock.h"
#include "utils.h"

class Server {
public:
    Server();
    ~Server();

    void run();

private:
    void createFile();
    void printFile();
    int findEmployeeIndex(int id);
    void updateEmployee(const Employee& emp);
    void startClients(int client_count);
    void handleClient(int client_id, HANDLE hPipe);

    bool lockForRead(int client_id, int employeeId);
    bool lockForWrite(int client_id, int employeeId);
    bool unlockRecord(int client_id, int employeeId);

    Employee readRecord(int client_id, int employeeId);
    bool writeRecord(int client_id, int employeeId, const Employee& emp);

    std::string filename;
    std::vector<Employee> employees;
    std::map<int, RecordLock*> record_locks;
    std::mutex map_mutex;
    std::atomic<bool> running;

    std::mutex lock_mutex;
    std::map<int, int> write_locks;
    std::map<int, std::set<int>> read_locks;

    const std::string PIPE_NAME_BASE = "\\\\.\\pipe\\lab5_pipe_";
};

#endif
