// RUN: qparse %s.qasm

// This file tests semantic type enforcement during measurements.
// In OpenQASM, the target of a measurement must be a classical
// register (creg). This test verifies that the compiler blocks
// attempts to measure a quantum state directly into another qubit.

OPENQASM 2.0;

qreg q[1];
creg c[1];
measure q[0] -> q[0];
