#ifndef EXESIZES_H
#define EXESIZES_H

#include <iostream>
#include <vector>
using namespace std;

struct ListN {
    int value;
    ListN* next;
    ListN(int val) : value(val), next(nullptr) {}
    ListN(int val, ListN* next) : value(val), next(next) {} 
};

vector<int> fibo(int n);
bool palindrome(int n);
ListN* Reverse(ListN* head);
void PrintResult(int n1, int n2, ListN* head);
ListN* inputList();

#endif