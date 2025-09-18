
cmake_minimum_required (VERSION 3.8)

project ("Lab_1")

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL" CACHE STRING "" FORCE)
endif()

add_executable(Main main.cpp)
add_executable(Creator creator.cpp)
add_executable(Reporter reporter.cpp)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_target_properties(Creator Reporter Main PROPERTIES CXX_STANDARD 20)
endif()


