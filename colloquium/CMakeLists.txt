
cmake_minimum_required (VERSION 3.8)
project ("kollokv_1")

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL" CACHE STRING "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

add_executable(Main main.cpp "exesizes.cpp")


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_target_properties(Main PROPERTIES CXX_STANDARD 20)
endif()

