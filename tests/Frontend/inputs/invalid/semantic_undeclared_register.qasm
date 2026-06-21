// RUN: qparse %s.qasm

// This file validates the semantic type-checking of the compiler.
// It ensures that attempting to apply a quantum gate to a register
// that has not been previously declared results in a precise 
// semantic error regarding the unknown identifier.

OPENQASM 2.0;

qreg q[1];
h r[0];
