// RUN: ./build/qparse %s --emit-mlir | ./llvm-project/build/bin/FileCheck %s

// This file ensures that the compiler correctly handles an empty
// OpenQASM program, generating a valid, empty MLIR module and circuit function.

OPENQASM 2.0;
include "qelib1.inc";

// CHECK: func.func @circuit() {
// CHECK: return
// CHECK: }
