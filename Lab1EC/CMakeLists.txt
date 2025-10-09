
cmake_minimum_required (VERSION 3.8)

project ("Lab_1")

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL" CACHE STRING "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

add_executable(Main main.cpp  "employee.cpp")
add_executable(Creator creator.cpp  "employee.cpp")
add_executable(Reporter reporter.cpp  "employee.cpp")


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_target_properties(Creator Reporter Main PROPERTIES CXX_STANDARD 20)
endif()


include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(MyTest tests.cpp  employee.cpp)
target_link_libraries(MyTest PRIVATE gtest gtest_main)
include(GoogleTest)
gtest_discover_tests(MyTest)

add_custom_target(check ALL
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS MyTest
)

add_dependencies(Main check)

