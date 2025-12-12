cmake_minimum_required(VERSION 3.8)

project(lab4)

add_executable(receiver receiver.cpp)
add_executable(sender sender.cpp)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL" CACHE STRING "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()


