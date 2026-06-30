# Quantum Compiler: OpenQASM 2.0 Subset to MLIR

<p>
  <img src="https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white" alt="C++" />
  <img src="https://img.shields.io/badge/LLVM_%2F_MLIR-1f2532?style=for-the-badge" alt="LLVM/MLIR" />
  <img src="https://img.shields.io/badge/CMake-064F8C?style=for-the-badge&logo=cmake&logoColor=white" alt="CMake" />
  <img src="https://img.shields.io/badge/Python-3776AB?style=for-the-badge&logo=python&logoColor=white" alt="Python" />
  <img src="https://img.shields.io/badge/Qiskit-6929C4?style=for-the-badge&logo=qiskit&logoColor=white" alt="Qiskit" />
</p>

This project is a quantum compiler developed as a thesis project. It takes as input code written in a subset of **OpenQASM 2.0**, generates an Abstract Syntax Tree (AST), lowers it into the **MLIR** framework (creating a custom `quantum` dialect), and performs circuit optimization passes (e.g., auto-inverse gate cancellation). Finally, it re-emits optimized OpenQASM code and verifies semantic equivalence via simulation using Qiskit.

## 🛠️ System Prerequisites
To compile the project and the underlying LLVM/MLIR framework, ensure you have the following packages installed:

```bash
sudo apt update
sudo apt install build-essential clang cmake ninja-build git python3 python3-pip python3-venv
```

⚠️ **Note: This project heavily depends on LLVM and MLIR. Cloning and compiling LLVM requires time, RAM, and significant disk space (at least 20GB of free space is recommended).**

## 🚀 Environment Setup

### 1. Download and Compile LLVM/MLIR
From the root of this project, run the following commands:

#### Clone the official LLVM repository
```bash
git clone https://github.com/llvm/llvm-project.git
cd llvm-project
```

#### Configure the build with CMake (Release mode, X86 target, enable MLIR)
```bash
cmake -B build -G Ninja ../llvm \
   -DLLVM_ENABLE_PROJECTS=mlir \
   -DLLVM_BUILD_EXAMPLES=ON \
   -DLLVM_TARGETS_TO_BUILD="X86" \
   -DCMAKE_BUILD_TYPE=Release \
   -DLLVM_ENABLE_ASSERTIONS=ON \
   -DCMAKE_C_COMPILER=clang \
   -DCMAKE_CXX_COMPILER=clang++ \
   -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

#### Start the build (This may take from 30 minutes to several hours)
```bash
ninja
```

### 2. Python Environment Setup (For Verification and Simulation)
The test suite and simulations to verify the equivalence of optimized circuits are written in Python using Qiskit. From the root of the project:

```bash
python3 -m venv quantum_env
source quantum_env/bin/activate
pip install -r requirements.txt
```

### 3. Compiling the Local Project (Quantum Compiler)
Once the LLVM framework is built, you can compile the actual frontend of the quantum compiler. The build system is managed via CMake and Ninja. Generates the build directory and the Ninja configuration files.
```bash
cmake -B build -G Ninja
```

## OpenQASM 2.0 Subset Grammar (EBNF)

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

The parser accepts the gates above syntactically. The optimization pipeline currently targets the subset `h`, `x`, `y`, `z`, `cx`, `rx`, `ry`, `rz`; `s`, `t`, and `measure` remain recognized by the parser but are not part of the optimization path.

## ⚛️ MLIR Quantum Dialect (SSA-style)

This compiler models quantum circuits internally using a custom MLIR dialect (`quantum`). To facilitate safe, deterministic optimization passes (like cancelling adjacent $H$ gates), the dialect enforces strict **Value Semantics (SSA-style)**. In this paradigm, qubits are not updated "in place" in a global memory array. Instead, every quantum gate consumes an input qubit value and produces a *new* qubit value representing the updated state.

### Design Philosophy & Constraints

The chosen architecture is specifically designed around the mathematical and physical constraints of quantum circuits:

#### Value Semantics & No-Cloning
In a standard SSA representation, a value could theoretically be used multiple times. However, quantum mechanics dictates that unknown states cannot be duplicated (No-Cloning Theorem). By modeling gates to consume an input qubit and return a *new* qubit, the dialect natively reflects state evolution over time. Attempting to branch a quantum state (e.g., `%q1 = quantum.h %q0; %q2 = quantum.h %q0`) breaks the linear flow of the SSA chain and can be structurally intercepted by MLIR verifiers to prevent unphysical cloning.

#### Topological Adjacency for Pattern Matching
By avoiding global array mutations and using direct SSA value consumption, the Intermediate Representation becomes a strict dataflow graph. This design anticipates optimization passes (like auto-inverse cancellation). Recognizing that two CNOT gates are structurally adjacent does not require checking if they act on variables with the same string names; it only requires matching the topological pattern `op(op(x))`, making optimization safe and trivial through MLIR pattern rewriting.

### Types

| Type | Description |
|---|---|
| `!quantum.qubit` | Represents a single, versioned quantum bit. |
| `!quantum.qreg<N>` | Represents a register of `N` qubits. |

### Supported Operations

The dialect models the core gate set from the OpenQASM 2.0 subset:

#### Unary Gates (No Parameters)
| Operation | Signature | Description |
|---|---|---|
| `quantum.h` | `(qubit) -> qubit` | Hadamard gate |
| `quantum.x` | `(qubit) -> qubit` | Pauli-X gate |
| `quantum.y` | `(qubit) -> qubit` | Pauli-Y gate |
| `quantum.z` | `(qubit) -> qubit` | Pauli-Z gate |
| `quantum.s` | `(qubit) -> qubit` | Phase (S) gate |
| `quantum.t` | `(qubit) -> qubit` | $\pi/8$ (T) gate |

#### Parameterized Unary Gates
| Operation | Signature | Description |
|---|---|---|
| `quantum.rx` | `(f64, qubit) -> qubit` | Rotation around X-axis |
| `quantum.ry` | `(f64, qubit) -> qubit` | Rotation around Y-axis |
| `quantum.rz` | `(f64, qubit) -> qubit` | Rotation around Z-axis |

#### Binary Gates
| Operation | Signature | Description |
|---|---|---|
| `quantum.cx` | `(qubit, qubit) -> qubit, qubit` | Controlled-NOT (CNOT) gate |

#### Measurement and Extraction
| Operation | Signature | Description |
|---|---|---|
| `quantum.measure` | `(qubit) -> qubit, i1` | Measures a qubit, yielding the updated qubit and a classical bit |
| `quantum.extract` | `(qreg, i64) -> qubit` | Extracts an SSA qubit value from a `qreg` array |

### Example IR
A simple quantum snippet like `h q[0];` embedded in a function becomes:
```mlir
func.func @circuit(%qreg: !quantum.qreg<1>) {
  %q_0 = quantum.extract %qreg[0] : !quantum.qreg<1> -> !quantum.qubit
  %q_1 = quantum.h %q_0 : !quantum.qubit -> !quantum.qubit
  func.return
}
```

## 💻 Usage
*This section will be updated as the compiler development progresses*

### 🔍 Frontend (Parser & AST)
Use the `qparse` tool to perform lexical and syntactic analysis of OpenQASM 2.0 source files.

**Validate syntax:** Checks if the provided file is a valid OpenQASM subset. It will halt and report precise line/column diagnostics on lexical or syntax errors.
```bash
qparse input.qasm
```

```bash
===== Lexical Analysis of tests/Frontend/inputs/valid/comprehensive.qasm =====
Line 1     Col 1    | Keyword: OPENQASM
Line 1     Col 10   | Float: 2
Line 1     Col 13   | Symbol: ';'
Line 2     Col 1    | Keyword: include
Line 2     Col 9    | String: "qelib1.inc"
Line 2     Col 21   | Symbol: ';'
Line 5     Col 1    | Keyword: qreg
Line 5     Col 6    | Identifier: q
Line 5     Col 7    | Symbol: '['
Line 5     Col 8    | Integer: 3
Line 5     Col 9    | Symbol: ']'
Line 5     Col 10   | Symbol: ';'
...
```

**Dump the Abstract Syntax Tree:** Parses the input file and prints the hierarchical, typed AST representation.
```bash
qparse input.qasm --dump-ast
```

```bash
===== Parsing and AST Dump of tests/Frontend/inputs/valid/comprehensive.qasm =====
ProgramAST
  OpenQASMVersion: 2
  Include: "qelib1.inc"
  QRegDecl 'q' [3]
  ...
```

### ⚛️ MLIR Dialect & Verification
The `quantum-opt` tool acts as the MLIR driver for the custom `quantum` dialect. It is used to parse, verify, and transform quantum IR files (`.mlir`).

**Verify Semantic Diagnostics:** 
Run the tool with `--verify-diagnostics` to ensure that custom verifiers correctly block invalid operations (e.g., passing wrong types to a gate):
```bash
./build/quantum-opt input.mlir --verify-diagnostics
```

**Round-Trip Parsing & Printing:** 
Combine `quantum-opt` with LLVM's `FileCheck` to verify that the dialect parses and prints perfectly without loss of information:
```bash
.quantum-opt input.mlir | ./llvm-project/build/bin/FileCheck input.mlir
```

### ⚙️ Optimization (Planned usage)
Use the `qcc` tool to optimize a `qasm` file.
```bash
qcc input.qasm --optimize -emit=qasm -o output.qasm
```

## 📋 Project Roadmap
The development of this compiler is divided into the following milestones:
- [x] **Task 1:** Parser and AST generation from OpenQASM subset.
- [x] **Task 2:** Custom `quantum` dialect definition in MLIR (ODS/TableGen).
- [ ] **Task 3:** MLIRGen (AST visitor to SSA-style Intermediate Representation).
- [ ] **Task 4:** Optimization Pass - Auto-inverse gate cancellation (e.g., H-H).
- [ ] **Task 5:** Backend - Re-emission of optimized OpenQASM code.
- [ ] **Task 6:** End-to-end equivalence verification via Qiskit Aer simulation.
