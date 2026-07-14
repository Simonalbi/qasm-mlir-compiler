// RUN: ./build/qparse %s --emit-mlir | ./llvm-project/build/bin/FileCheck %s

// This file validates the compile-time evaluation of symbolic math expressions.
// It ensures that OpenQASM constants (e.g., pi/2, -pi/4) are correctly evaluated
// by the AST visitor and lowered into concrete F64 attributes in the MLIR.

OPENQASM 2.0;
include "qelib1.inc";

qreg q[1];
// CHECK: %[[Q0:.*]] = quantum.extract %arg0[0] : <1> -> !quantum.qubit

rx(pi/2) q[0];
// CHECK: %[[Q1:.*]] = quantum.rx 1.570796{{.*}}, %[[Q0]] : !quantum.qubit -> !quantum.qubit

ry(-pi/4) q[0];
// CHECK: %[[Q2:.*]] = quantum.ry -0.785398{{.*}}, %[[Q1]] : !quantum.qubit -> !quantum.qubit

rz(2*pi/4) q[0];
// CHECK: %[[Q3:.*]] = quantum.rz 1.570796{{.*}}, %[[Q2]] : !quantum.qubit -> !quantum.qubit

// CHECK: return %[[Q3]] : !quantum.qubit
