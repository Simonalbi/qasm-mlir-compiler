// RUN: ./build/quantum-opt %s --verify-diagnostics

// This file tests that the TableGen-generated verifier
// correctly identifies invalid types passed to the 'quantum.cx'
// operation. Specifically, it checks that if we pass a quantum
// register (Qreg) instead of a single qubit to the CNOT operation,
// the verifier will produce an error indicating the type mismatch.

func.func @circuit(%qreg: !quantum.qreg<2>) -> (!quantum.qubit, !quantum.qubit) {
  %q0 = quantum.extract %qreg[0] : !quantum.qreg<2> -> !quantum.qubit
  
  // expected-error @+1 {{custom op 'quantum.cx' invalid kind of type specified: expected quantum.qubit, but found '!quantum.qreg<2>'}}
  %out1, %out2 = quantum.cx %q0, %qreg : !quantum.qubit, !quantum.qreg<2> -> !quantum.qubit, !quantum.qubit
  
  return %out1, %out2 : !quantum.qubit, !quantum.qubit
}
