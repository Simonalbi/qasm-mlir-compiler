// RUN: qparse %s

// This file tests the syntactical robustness of the parser.
// It checks that the parser correctly identifies and reports errors
// for missing semicolons at the end of statements, as well as the
// use of invalid operators (like '=>' instead of '->').

OPENQASM 2.0;

qreg q[1]
creg c[1];

measure q[0] => c[0];
