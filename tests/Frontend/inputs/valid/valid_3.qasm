OPENQASM 2.0;

include "qelib1.inc";

qreg my_qreg_1[5];
creg my_creg_2[5];

cx my_qreg_1[0], my_qreg_1[4];

