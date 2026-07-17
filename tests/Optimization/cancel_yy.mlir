// RUN: ./build/quantum-opt %s --quantum-cancel-inverse | ./llvm-project/build/bin/FileCheck %s

// This file verifies the auto-inverse cancellation optimization for the Pauli-Y gate.
// It ensures that consecutive Y gates on the same qubit are correctly eliminated.

func.func @circuit(%qreg: !quantum.qreg<1>) -> !quantum.qubit {
  // CHECK: %[[Q0:.*]] = quantum.extract %arg0[0]
  %0 = quantum.extract %qreg[0] : <1> -> !quantum.qubit
  
  // CHECK-NOT: quantum.y
  %1 = quantum.y %0 : !quantum.qubit -> !quantum.qubit
  %2 = quantum.y %1 : !quantum.qubit -> !quantum.qubit
  
  // CHECK: return %[[Q0]] : !quantum.qubit
  return %2 : !quantum.qubit
}
