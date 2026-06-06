OPENQASM 2.0;

qreg q[1]               // Error: missing semicolon
creg c[1];

measure q[0] => c[0];   // Error: '=>' instead of '->'
