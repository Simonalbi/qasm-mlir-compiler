// RUN: ./build/qparse %s --emit-mlir | ./llvm-project/build/bin/FileCheck %s

OPENQASM 2.0;
include "qelib1.inc";

// CHECK: func.func @circuit() {
// CHECK: return
// CHECK: }
