// RUN: ./build/qparse %s --emit-mlir | ./llvm-project/build/bin/FileCheck %s

// This file tests the strict SSA chain for single-qubit gates.
// It verifies that a sequence of consecutive Hadamard (h) gates correctly
// captures the output of the previous gate and reuses it, adhering to the
// No-Cloning theorem.

OPENQASM 2.0;
include "qelib1.inc";

qreg q[1];
// CHECK: %[[Q0:.*]] = quantum.extract %arg0[0] : <1> -> !quantum.qubit

h q[0];
// CHECK: %[[Q1:.*]] = quantum.h %[[Q0]] : !quantum.qubit -> !quantum.qubit

h q[0];
// CHECK: %[[Q2:.*]] = quantum.h %[[Q1]] : !quantum.qubit -> !quantum.qubit

// CHECK: return %[[Q2]] : !quantum.qubit
