#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <iostream>
#include <string>
#include "utils.h"
using namespace std;

struct Employee {
    int id;
    char name[10];
    double hours;
};

enum class OperationType {
    READ = OPERATION_READ,
    WRITE = OPERATION_WRITE,
    END = OPERATION_END
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
    bool valid_id = false;
    while (!valid_id) {
        cout << "Employee's id (1-" << MAX_EMPLOYEES << "): ";
        emp.id = GetValidIntInput();
        valid_id = IsValidEmployeeId(emp.id);
        if (!valid_id) {
            PrintError("Invalid employee ID! Must be between 1 and " +
                to_string(MAX_EMPLOYEES));
        }
    }

    string name;
    bool valid_name = false;
    while (!valid_name) {
        cout << "Employee's name (max 9 chars): ";
        name = GetValidStringInput();
        valid_name = IsValidEmployeeName(name);
        if (!valid_name) {
            PrintError("Name must be 1-9 characters!");
        }
    }
    strncpy_s(emp.name, sizeof(emp.name), name.c_str(), _TRUNCATE);

    cout << "Work hours: ";
    bool valid_hours = false;
    while (!valid_hours) {
        emp.hours = GetValidDoubleInput();
        valid_hours = IsValidWorkHours(emp.hours);
        if (!valid_hours) {
            PrintError("Work hours must be between 0 and 168!");
            cout << "Work hours: ";
        }
    }

    return in;
}

#endif
