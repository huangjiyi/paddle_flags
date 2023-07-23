cmake_minimum_required(VERSION 3.22)

project(paddle_flags)
set(CMAKE_CXX_STANDARD 17)

include_directories(src)
set(SOURCES test/flags_native_test_2.cc test/flags_native_test.cc src/flags_native.cc)

add_executable(test ${SOURCES})