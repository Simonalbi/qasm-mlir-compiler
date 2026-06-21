// RUN: qparse %s.qasm

// This file tests the lexical analysis phase of the parser.
// It verifies that an error is correctly emitted when an identifier
// starts with a number, which violates the OpenQASM 2.0 naming rules.

OPENQASM 2.0;
include "qelib1.inc";

qreg 1invalid_name[3];
creg b[3];

x a[0];
y a[1];
z a[2];
