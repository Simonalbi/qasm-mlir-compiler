// RUN: ./build/quantum-opt %s --quantum-cancel-inverse | ./llvm-project/build/bin/FileCheck %s

// This file verifies the auto-inverse cancellation optimization for the Pauli-X gate.
// It ensures that consecutive X gates on the same qubit are correctly eliminated.

func.func @circuit(%qreg: !quantum.qreg<1>) -> !quantum.qubit {
  // CHECK: %[[Q0:.*]] = quantum.extract %arg0[0]
  %0 = quantum.extract %qreg[0] : <1> -> !quantum.qubit
  
  // CHECK-NOT: quantum.x
  %1 = quantum.x %0 : !quantum.qubit -> !quantum.qubit
  %2 = quantum.x %1 : !quantum.qubit -> !quantum.qubit
  
  // CHECK: return %[[Q0]] : !quantum.qubit
  return %2 : !quantum.qubit
}
