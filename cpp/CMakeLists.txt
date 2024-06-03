cmake_minimum_required(VERSION 3.29.3)
project(Math-Expression-Evaluator, CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_FLAGS "${CMAKE_CRR_FLAGS} -Werror -Wall -Wextra -Wpedantic -Wconversion")
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)


add_executable(ExpressionEvaluator
    ./src/main.cpp)

target_include_directories(ExpressionEvaluator PRIVATE ./src)

target_compile_options(ExpressionEvaluator PRIVATE -O0 -std=c++23)

