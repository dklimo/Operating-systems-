cmake_minimum_required(VERSION 3.8)

project(lab4)

add_executable(receiver receiver.cpp "utils.cpp" "process.cpp")
add_executable(sender sender.cpp "utils.cpp" "process.cpp")

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL" CACHE STRING "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()


include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(MyTest tests.cpp utils.cpp process.cpp) 
target_link_libraries(MyTest PRIVATE gtest gtest_main)
include(GoogleTest)
gtest_discover_tests(MyTest)

add_custom_target(check ALL
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS MyTest
)

add_dependencies(receiver check)
add_dependencies(sender check)




