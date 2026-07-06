// RUN: ./build/quantum-opt %s | ./llvm-project/build/bin/FileCheck %s

// This file contains a semantically valid quantum circuit.
// The goal is to verify that the tool correctly reads the IR text,
// builds the circuit in memory, and prints it back verbatim without data loss
// (Round-Trip). FileCheck ensures that the output strictly matches
// the patterns specified in the CHECK directives line by line.

func.func @circuit(%qreg: !quantum.qreg<2>) -> (!quantum.qubit, !quantum.qubit) {
  // Test for qreg and extract
  // CHECK: %{{.*}} = quantum.extract %{{.*}}[0] : <2> -> !quantum.qubit
  %q0 = quantum.extract %qreg[0] : !quantum.qreg<2> -> !quantum.qubit
  
  // CHECK: %{{.*}} = quantum.extract %{{.*}}[1] : <2> -> !quantum.qubit
  %q1 = quantum.extract %qreg[1] : !quantum.qreg<2> -> !quantum.qubit

  // Test for simple unary gates (h)
  // CHECK: %{{.*}} = quantum.h %{{.*}} : !quantum.qubit -> !quantum.qubit
  %q0_h = quantum.h %q0 : !quantum.qubit -> !quantum.qubit

  // Test for binary gate (cnot)
  // CHECK: %{{.*}}, %{{.*}} = quantum.cx %{{.*}}, %{{.*}} : !quantum.qubit, !quantum.qubit -> !quantum.qubit, !quantum.qubit
  %q0_cx, %q1_cx = quantum.cx %q0_h, %q1 : !quantum.qubit, !quantum.qubit -> !quantum.qubit, !quantum.qubit

  // Test for parameterized unary gates (rx)
  // CHECK: %{{.*}} = quantum.rx 3.14{{.*}}, %{{.*}} : !quantum.qubit -> !quantum.qubit
  %q1_rx = quantum.rx 3.1415, %q1_cx : !quantum.qubit -> !quantum.qubit

  // Test for measurement
  // CHECK: %{{.*}}, %{{.*}} = quantum.measure %{{.*}} : !quantum.qubit -> !quantum.qubit, i1
  %q0_m, %bit = quantum.measure %q0_cx : !quantum.qubit -> !quantum.qubit, i1

  // CHECK: return %{{.*}}, %{{.*}} : !quantum.qubit, !quantum.qubit
  return %q0_m, %q1_rx : !quantum.qubit, !quantum.qubit
}
