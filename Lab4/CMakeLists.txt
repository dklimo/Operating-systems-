cmake_minimum_required(VERSION 3.8)

project(lab4)

add_executable(receiver receiver.cpp "utils.cpp" "process.cpp")
add_executable(sender sender.cpp "utils.cpp" "process.cpp")

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL" CACHE STRING "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()



