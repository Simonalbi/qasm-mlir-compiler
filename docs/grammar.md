# OpenQASM 2.0 Subset Grammar (EBNF)

This document formalizes the grammar for the subset of OpenQASM 2.0 supported by this compiler.

## Formal Definition

```ebnf
Program      ::= VersionDecl? Statement* EOF
VersionDecl  ::= "OPENQASM" "2.0" ";"

Statement    ::= IncludeDecl | QRegDecl | CRegDecl | GateCall | Measure
IncludeDecl  ::= "include" StringLiteral ";"

QRegDecl     ::= "qreg" Identifier "[" Integer "]" ";"
CRegDecl     ::= "creg" Identifier "[" Integer "]" ";"

Measure      ::= "measure" Argument "->" Argument ";"

GateCall     ::= GateName ( "(" Expression ")" )? ArgumentList ";"
GateName     ::= "h" | "x" | "y" | "z" | "s" | "t" | "rx" | "ry" | "rz" | "cx"

ArgumentList ::= Argument ( "," Argument )*
Argument     ::= Identifier "[" Integer "]"

Expression   ::= "-"? PrimaryExpr ( BinaryOp PrimaryExpr )?
PrimaryExpr  ::= Float | Integer | "pi"
BinaryOp     ::= "+" | "-" | "*" | "/"
```