#ifndef HELPER_H
#define HELPER_H

#include <iostream>
#include <string>
#include <vector> 

namespace helper {
    using std::cin;
    using std::cout;
    using std::string;
    using std::endl;  
    using std::vector; 
    constexpr int THREAD_SLEEP_WAIT = 100;
    constexpr int THREAD_SLEEP = 5;
    constexpr int SUCCESS_EXIT = 0;
    constexpr int FAILURE_EXIT = 1;
    constexpr int FORCED_WAIT = 10;
    constexpr int INPUT_BUFFER_CLEAR_SIZE = 10000;
}
int GetValidatedArraySize();
int GetValidatedThreadCount();

#endif 
