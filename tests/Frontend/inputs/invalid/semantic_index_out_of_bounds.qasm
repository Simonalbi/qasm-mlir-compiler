// RUN: qparse %s

// This file verifies that array boundary checks are correctly enforced.
// It tests whether the semantic analyzer catches out-of-bounds array
// accesses when trying to address a qubit index that exceeds the
// declared size of the quantum register.

OPENQASM 2.0;

qreg q[1];
h q[1];
