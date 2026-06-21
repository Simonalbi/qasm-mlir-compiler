// RUN: qparse %s.qasm

// This file tests the parser's flexibility with custom identifiers.
// It ensures that multi-qubit gates like CNOT function correctly
// even when addressing registers with long, non-standard names
// and disjoint, non-sequential array indices.

OPENQASM 2.0;
include "qelib1.inc";

qreg my_qreg_1[5];
creg my_creg_2[5];

cx my_qreg_1[0], my_qreg_1[4];
