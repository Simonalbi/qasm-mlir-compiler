OPENQASM 2.0;

include "qelib1.inc";

qreg q[2];

rx(1.5707) q[0];
ry(pi) q[1];
rz(0.5) q[0];
