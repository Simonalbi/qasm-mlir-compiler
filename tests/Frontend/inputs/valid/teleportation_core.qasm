// RUN: qparse %s.qasm

// This file models the core operations of a quantum teleportation protocol.
// It is used to verify that the AST correctly captures a sequential cascade
// of Hadamard and CNOT gates acting across multiple qubits, followed by
// concurrent measurement operations.

OPENQASM 2.0;
include "qelib1.inc";

qreg q[3];
creg c[2];

h q[1];
cx q[1], q[2];
cx q[0], q[1];
h q[0];

measure q[0] -> c[0];
measure q[1] -> c[1];
