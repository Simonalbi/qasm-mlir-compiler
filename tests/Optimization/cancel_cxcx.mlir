// RUN: ./build/quantum-opt %s --quantum-cancel-inverse | ./llvm-project/build/bin/FileCheck %s

// This file verifies the auto-inverse cancellation for the binary CNOT (CX) gate.
// It ensures that when two consecutive CX gates share the exact same control
// and target qubits, both gates are removed and the two SSA chains are bypassed.

func.func @circuit(%qreg: !quantum.qreg<2>) -> (!quantum.qubit, !quantum.qubit) {
  // CHECK: %[[C0:.*]] = quantum.extract %arg0[0]
  // CHECK: %[[T0:.*]] = quantum.extract %arg0[1]
  %0 = quantum.extract %qreg[0] : <2> -> !quantum.qubit
  %1 = quantum.extract %qreg[1] : <2> -> !quantum.qubit
  
  // CHECK-NOT: quantum.cx
  %ctrl_out, %target_out = quantum.cx %0, %1 : !quantum.qubit, !quantum.qubit -> !quantum.qubit, !quantum.qubit
  %c2, %t2 = quantum.cx %ctrl_out, %target_out : !quantum.qubit, !quantum.qubit -> !quantum.qubit, !quantum.qubit
  
  // CHECK: return %[[C0]], %[[T0]] : !quantum.qubit, !quantum.qubit
  return %c2, %t2 : !quantum.qubit, !quantum.qubit
}
