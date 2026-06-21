cmake_minimum_required(VERSION 3.15)
project(qasm-mlir-compiler CXX)

# LLVM C++17 Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

# MLIR & LLVM Setup
set(LLVM_DIR "${CMAKE_SOURCE_DIR}/llvm-project/build/lib/cmake/llvm" CACHE PATH "Path to LLVM CMake config")
set(MLIR_DIR "${CMAKE_SOURCE_DIR}/llvm-project/build/lib/cmake/mlir" CACHE PATH "Path to MLIR CMake config")

find_package(MLIR REQUIRED CONFIG)
message(STATUS "Using MLIRConfig.cmake in: ${MLIR_DIR}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

list(APPEND CMAKE_MODULE_PATH "${MLIR_CMAKE_DIR}")
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")

include(TableGen)
include(AddLLVM)
include(AddMLIR)
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${MLIR_INCLUDE_DIRS})

# Include directory for tablegen generated headers
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Header files
include_directories(include)

# Frontend library
add_library(Frontend 
    lib/Frontend/Lexer.cpp
    lib/Frontend/Parser.cpp
    lib/Frontend/AST.cpp
)

target_include_directories(Frontend PUBLIC include)

# MLIR Libraries
add_library(QuantumDialect 
    lib/Dialect/QuantumDialect.cpp
)
add_dependencies(QuantumDialect QuantumIncGen)
target_include_directories(QuantumDialect PUBLIC include)
target_link_libraries(QuantumDialect PUBLIC 
    MLIRIR
    MLIRSupport
    MLIRSideEffectInterfaces
)

# Executables
add_executable(qparse tools/qparse/qparse.cpp)
target_link_libraries(qparse PRIVATE Frontend)

add_executable(quantum-opt tools/quantum-opt/quantum-opt.cpp)
target_link_libraries(quantum-opt PRIVATE 
    QuantumDialect
    MLIRFuncDialect
    MLIRParser
    MLIRPass
    MLIRTransforms
    MLIRSupport
    MLIROptLib
)
set(LLVM_TARGET_DEFINITIONS include/Dialect/Quantum.td)
mlir_tablegen(QuantumDialect.h.inc -gen-dialect-decls -dialect=quantum)
mlir_tablegen(QuantumDialect.cpp.inc -gen-dialect-defs -dialect=quantum)
mlir_tablegen(QuantumTypes.h.inc -gen-typedef-decls -typedefs-dialect=quantum)
mlir_tablegen(QuantumTypes.cpp.inc -gen-typedef-defs -typedefs-dialect=quantum)
mlir_tablegen(QuantumOps.h.inc -gen-op-decls)
mlir_tablegen(QuantumOps.cpp.inc -gen-op-defs)

add_public_tablegen_target(QuantumIncGen)