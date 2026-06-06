OPENQASM 2.0;

include "qelib1.inc";

qreg 1invalid_name[3];  // Error: identifier starting with a number
creg b[3] @;            // Error: invalid symbol '@'

x a[0];
y a[1];
z a[2];
