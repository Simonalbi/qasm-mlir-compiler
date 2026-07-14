// RUN: qparse %s

// This file contains a standard, valid quantum circuit implementation.
// It serves as a baseline test to ensure that the parser correctly
// processes register declarations, basic unary and binary gates (H, CX),
// parameterized rotations, and final classical measurements.

OPENQASM 2.0;
include "qelib1.inc";

qreg q[2];
creg c[2];

h q[0];
cx q[0], q[1];

rx(3.14) q[0];
rz(pi) q[1];

measure q[0] -> c[0];
