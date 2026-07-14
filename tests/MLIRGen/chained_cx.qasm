// RUN: ./build/qparse %s --emit-mlir | ./llvm-project/build/bin/FileCheck %s

// This file verifies the correct propagation of SSA values across consecutive
// CNOT (cx) gates. It ensures that both the control and target qubits are
// consumed and their updated states are correctly passed to the next gate.

OPENQASM 2.0;
include "qelib1.inc";

qreg q[2];
// CHECK: %[[Q0:.*]] = quantum.extract %arg0[0] : <2> -> !quantum.qubit
// CHECK: %[[Q1:.*]] = quantum.extract %arg0[1] : <2> -> !quantum.qubit

cx q[0], q[1];
// CHECK: %[[Q0_1:.*]], %[[Q1_1:.*]] = quantum.cx %[[Q0]], %[[Q1]] : !quantum.qubit, !quantum.qubit -> !quantum.qubit, !quantum.qubit

cx q[0], q[1];
// CHECK: %[[Q0_2:.*]], %[[Q1_2:.*]] = quantum.cx %[[Q0_1]], %[[Q1_1]] : !quantum.qubit, !quantum.qubit -> !quantum.qubit, !quantum.qubit

// CHECK: return %[[Q0_2]], %[[Q1_2]] : !quantum.qubit, !quantum.qubit
