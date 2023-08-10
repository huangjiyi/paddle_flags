cmake_minimum_required(VERSION 3.22)

project(paddle_flags)
set(CMAKE_CXX_STANDARD 17)

include_directories(src)
set(SOURCES 
    test/flags_test_2.cc 
    test/flags_test.cc 
    src/flags.cc)

add_executable(test ${SOURCES})