// RUN: qparse %s.qasm

// This file verifies the lexer's ability to handle malformed strings.
// Specifically, it ensures that an unterminated string literal 
// (missing the closing quotation mark) is caught and reported
// as a lexical error before parsing continues.

OPENQASM 2.0;
include "qelib1.inc;

qreg a[3];
creg b[3];
