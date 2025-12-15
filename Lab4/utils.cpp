#include "utils.h"
#include <windows.h>

void SetColor(ConsoleColor color) {
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdOut, (WORD)color);
}