#include <fstream>
#include <iostream>
#include <string>
#ifndef EMPLOYEE_H
#define EMPLOYEE_H

using namespace std;

struct employee {
    int num;
    char name[10];
    double hours;

    bool readFromFile(istream& fin) {
        return fin.read(reinterpret_cast<char*>(&num), sizeof(num)) && fin.read(name, sizeof(name)) 
            && fin.read(reinterpret_cast<char*>(&hours), sizeof(hours));
    }
    bool writeToFile(ostream& fout) {
        return fout.write(reinterpret_cast<char*>(&num), sizeof(num)) && fout.write(name, sizeof(name))
            && fout.write(reinterpret_cast<char*>(&hours), sizeof(hours));
    }
};
void writeBinaryFile(const string& filename);
void writeReport(const string& filename);

#endif
