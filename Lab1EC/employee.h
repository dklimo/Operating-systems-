#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#ifndef EMPLOYEE_H
#define EMPLOYEE_H

struct employee {
    int num;
    char name[10];
    double hours;
};
void writeBinaryFile(const std::string& filename);
void writeReport(const std::string& filename);

#endif