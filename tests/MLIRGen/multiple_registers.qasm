// RUN: ./build/qparse %s --emit-mlir | ./llvm-project/build/bin/FileCheck %s

OPENQASM 2.0;
include "qelib1.inc";

// CHECK: func.func @circuit(%arg0: !quantum.qreg<2>, %arg1: !quantum.qreg<1>) -> (!quantum.qubit, !quantum.qubit, !quantum.qubit)
qreg q[2];
qreg p[1];

// CHECK: %[[Q0:.*]] = quantum.extract %arg0[0] : <2> -> !quantum.qubit
// CHECK: %[[Q1:.*]] = quantum.extract %arg0[1] : <2> -> !quantum.qubit
// CHECK: %[[P0:.*]] = quantum.extract %arg1[0] : <1> -> !quantum.qubit

h q[1];
// CHECK: %[[Q1_1:.*]] = quantum.h %[[Q1]] : !quantum.qubit -> !quantum.qubit

x p[0];
// CHECK: %[[P0_1:.*]] = quantum.x %[[P0]] : !quantum.qubit -> !quantum.qubit

cx q[1], p[0];
// CHECK: %[[Q1_2:.*]], %[[P0_2:.*]] = quantum.cx %[[Q1_1]], %[[P0_1]] : !quantum.qubit, !quantum.qubit -> !quantum.qubit, !quantum.qubit

// CHECK: return %[[Q0]], %[[Q1_2]], %[[P0_2]] : !quantum.qubit, !quantum.qubit, !quantum.qubit
