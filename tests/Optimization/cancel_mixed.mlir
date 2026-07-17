// RUN: ./build/quantum-opt %s --quantum-cancel-inverse | ./llvm-project/build/bin/FileCheck %s

// This file verifies that the optimization pass works correctly when multiple
// different auto-inverse pairs (e.g., H-H and X-X) exist in the same circuit,
// while correctly preserving any non-cancelled operations (e.g. a single Z gate) in between.

func.func @circuit(%qreg: !quantum.qreg<1>) -> !quantum.qubit {
  // CHECK: %[[Q0:.*]] = quantum.extract %arg0[0]
  %0 = quantum.extract %qreg[0] : <1> -> !quantum.qubit
  
  // H-H cancels
  %1 = quantum.h %0 : !quantum.qubit -> !quantum.qubit
  %2 = quantum.h %1 : !quantum.qubit -> !quantum.qubit
  
  // CHECK-NOT: quantum.h
  
  // Z remains
  // CHECK: %[[Z:.*]] = quantum.z %[[Q0]]
  %3 = quantum.z %2 : !quantum.qubit -> !quantum.qubit
  
  // X-X cancels
  %4 = quantum.x %3 : !quantum.qubit -> !quantum.qubit
  %5 = quantum.x %4 : !quantum.qubit -> !quantum.qubit
  
  // CHECK-NOT: quantum.x
  
  // CHECK: return %[[Z]] : !quantum.qubit
  return %5 : !quantum.qubit
}
