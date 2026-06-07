OPENQASM 2.0;
include "qelib1.inc";

// Registers declaration
qreg q[3];
creg c[3];

// 1. Single-qubit basic gate with no parameters
h q[0];
x q[1];
y q[2];
z q[0];
s q[1];
t q[2];

// 2. Two-qubit gate (creating entanglement)
cx q[0], q[1];
cx q[1], q[2];

// 3. Parameterized gates with various expressions
rx(pi) q[0];       // Simple constant
ry(pi/2) q[1];     // Binary operator (division)
rz(-pi/4) q[2];    // Unary minus combined with division
rx(1.5708) q[0];   // Literal float

// 4. Final measurements
measure q[0] -> c[0];
measure q[1] -> c[1];
measure q[2] -> c[2];