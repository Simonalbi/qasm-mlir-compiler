// RUN: qparse %s.qasm

// This file specifically tests the parsing of parameterized gates.
// It verifies that the frontend correctly evaluates parameters containing
// floating-point literals, as well as the special mathematical
// constant 'pi' within rotation gates (rx, ry, rz).

OPENQASM 2.0;
include "qelib1.inc";

qreg q[2];

rx(1.5707) q[0];
ry(pi) q[1];
rz(0.5) q[0];
