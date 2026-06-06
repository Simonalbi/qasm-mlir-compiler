OPENQASM 2.0;

include "qelib1.inc";

// Define reg
qreg q[2];
creg c[2];

h q[0];
cx q[0], q[1];

// Rotations with float and pi
rx(3.14) q[0];
rz(pi) q[1];

measure q[0] -> c[0];
