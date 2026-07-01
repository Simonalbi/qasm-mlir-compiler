// RUN: ./build/quantum-opt %s --quantum-verify-no-cloning --verify-diagnostics

// This file tests that the custom No-Cloning Verifier Pass
// correctly identifies and prevents the unphysical cloning of qubits.
// Specifically, it verifies that a single qubit value cannot be used
// as input to multiple quantum operations, and cannot be discarded
// without being used.

func.func @test_cloning(%qreg: !quantum.qreg<2>) {
  // Extract a single qubit. This qubit (%q0) is used twice (cloning violation).
  // expected-error @+1 {{No-Cloning theorem violated: a qubit value must be used exactly once.}}
  %q0 = quantum.extract %qreg[0] : !quantum.qreg<2> -> !quantum.qubit
  
  // The same qubit %q0 is passed to both H and X gates.
  // Moreover, %q_out1 and %q_out2 are never used afterwards (leak violation).
  // expected-error @+1 {{No-Cloning theorem violated: a qubit value must be used exactly once.}}
  %q_out1 = quantum.h %q0 : !quantum.qubit -> !quantum.qubit
  // expected-error @+1 {{No-Cloning theorem violated: a qubit value must be used exactly once.}}
  %q_out2 = quantum.x %q0 : !quantum.qubit -> !quantum.qubit

  func.return
}

// expected-error @+1 {{No-Cloning theorem violated: a block argument qubit must be used exactly once.}}
func.func @test_block_arg_cloning(%q0: !quantum.qubit) {
  // The block argument %q0 is passed to both H and X gates (cloning).
  // expected-error @+1 {{No-Cloning theorem violated: a qubit value must be used exactly once.}}
  %q1 = quantum.h %q0 : !quantum.qubit -> !quantum.qubit
  // expected-error @+1 {{No-Cloning theorem violated: a qubit value must be used exactly once.}}
  %q2 = quantum.x %q0 : !quantum.qubit -> !quantum.qubit

  func.return
}
