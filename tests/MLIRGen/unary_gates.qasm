// RUN: ./build/qparse %s --emit-mlir | ./llvm-project/build/bin/FileCheck %s

OPENQASM 2.0;
include "qelib1.inc";

qreg q[1];
// CHECK: %[[Q0:.*]] = quantum.extract %arg0[0] : <1> -> !quantum.qubit

h q[0];
// CHECK: %[[Q1:.*]] = quantum.h %[[Q0]] : !quantum.qubit -> !quantum.qubit

x q[0];
// CHECK: %[[Q2:.*]] = quantum.x %[[Q1]] : !quantum.qubit -> !quantum.qubit

// CHECK: return %[[Q2]] : !quantum.qubit
