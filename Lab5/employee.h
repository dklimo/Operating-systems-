#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <iostream>
#include <string>
using namespace std;

struct Employee {
    int id;
    char name[10];
    double hours;
};

enum class OperationType {
    READ = 1,
    WRITE = 2,
    END = 3
};

struct Request {
    OperationType operationType;
    int employeeId;
    Employee emp;
};

struct Response {
    bool success;
    Employee emp;
};

inline ostream& operator<<(ostream& out, const Employee& emp) {
    out << "Employee's id: " << emp.id << endl;
    out << "Employee's name: " << emp.name << endl;
    out << "Work hours: " << emp.hours << endl;
    return out;
}

inline istream& operator>>(istream& in, Employee& emp) {
    cout << "Employee's id: ";
    in >> emp.id;
    cout << "Employee's name (max 9 chars): ";
    in >> emp.name;
    cout << "Work hours: ";
    in >> emp.hours;
    return in;
}

#endif