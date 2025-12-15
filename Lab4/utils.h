#ifndef UTILS_H
#define UTILS_H
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

constexpr int MAX_MESSAGE_LENGTH = 20;
constexpr DWORD WAIT_TIMEOUT_MS = 1000; 
constexpr int ERROR_EXIT = 1;
constexpr int SUCCESS_EXIT = 0;

enum ConsoleColor {
    GREEN = 2,
    YELLOW = 6,
    RED = 4,
    WHITE = 7
};

void SetColor(ConsoleColor color);

#endif