// RUN: ./build/quantum-opt %s --quantum-cancel-inverse | ./llvm-project/build/bin/FileCheck %s

// This negative test ensures that the optimization pass does NOT erroneously cancel gates.
// It tests cases like consecutive but mismatched gates (e.g., H followed by X)
// and consecutive CX gates where the control and target wires are swapped.

func.func @circuit(%qreg: !quantum.qreg<2>) -> (!quantum.qubit, !quantum.qubit) {
  // CHECK: %[[C0:.*]] = quantum.extract %arg0[0]
  // CHECK: %[[T0:.*]] = quantum.extract %arg0[1]
  %0 = quantum.extract %qreg[0] : <2> -> !quantum.qubit
  %1 = quantum.extract %qreg[1] : <2> -> !quantum.qubit
  
  // 1. Different gates
  // CHECK: %[[H:.*]] = quantum.h %[[C0]]
  // CHECK: %[[X:.*]] = quantum.x %[[H]]
  %2 = quantum.h %0 : !quantum.qubit -> !quantum.qubit
  %3 = quantum.x %2 : !quantum.qubit -> !quantum.qubit
  
  // 2. CX with swapped wires (control becomes target, target becomes control)
  // This does NOT cancel out physically!
  // CHECK: %[[CX1_C:.*]], %[[CX1_T:.*]] = quantum.cx %[[X]], %[[T0]]
  // CHECK: %[[CX2_C:.*]], %[[CX2_T:.*]] = quantum.cx %[[CX1_T]], %[[CX1_C]]
  %c1, %t1 = quantum.cx %3, %1 : !quantum.qubit, !quantum.qubit -> !quantum.qubit, !quantum.qubit
  %c2, %t2 = quantum.cx %t1, %c1 : !quantum.qubit, !quantum.qubit -> !quantum.qubit, !quantum.qubit
  
  // CHECK: return %[[CX2_C]], %[[CX2_T]] : !quantum.qubit, !quantum.qubit
  return %c2, %t2 : !quantum.qubit, !quantum.qubit
}
