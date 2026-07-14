// RUN: ./build/qparse %s --emit-mlir | ./llvm-project/build/bin/FileCheck %s

// This file verifies the generation of measurement operations.
// It checks that the quantum.measure operation correctly produces both
// a classical bit (i1) and the updated (collapsed) qubit state.

OPENQASM 2.0;
include "qelib1.inc";

qreg q[1];
creg c[1];
// CHECK: %[[Q0:.*]] = quantum.extract %arg0[0] : <1> -> !quantum.qubit

h q[0];
// CHECK: %[[Q1:.*]] = quantum.h %[[Q0]] : !quantum.qubit -> !quantum.qubit

measure q[0] -> c[0];
// CHECK: %[[Q2:.*]], %[[BIT:.*]] = quantum.measure %[[Q1]] : !quantum.qubit -> !quantum.qubit, i1

// CHECK: return %[[Q2]] : !quantum.qubit
