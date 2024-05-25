cmake_minimum_required(VERSION 3.29.3)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_FLAGS "${CMAKE_CRR_FLAGS} -Werror -Wall -Wextra -Wpedantic -Wconversion")

project(Math-Expression-Evaluator, CXX)

add_executable(Expression_Evaluator
    ./src/main.cpp)

target_compile_options(Expression_Evaluator PRIVATE -O0 -std=c++23)

