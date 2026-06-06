cmake_minimum_required(VERSION 3.15)
project(qasm-mlir-compiler CXX)

# LLVM C++17 Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Header files
include_directories(include)

# Frontend library
add_library(Frontend lib/Frontend/Lexer.cpp)

# Executables
add_executable(qparse tools/qparse/qparse.cpp)

# Linking
target_link_libraries(qparse PRIVATE Frontend)