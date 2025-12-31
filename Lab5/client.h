#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <windows.h>
#include "employee.h"

class Client {
public:
    Client(int id);
    ~Client();
    void run();

private:
    void modifyRecord();
    void readRecord();
    bool connectToServer();
    void disconnectFromServer();
    bool sendRequest(const Request& request);
    bool receiveResponse(Response& response);

    int client_id;
    std::string pipeName;
    HANDLE hPipe;
    bool connected;
};

#endif