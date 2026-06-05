# Quantum Compiler: OpenQASM 2.0 to MLIR
This project is a quantum compiler developed as a thesis project. It takes as input code written in a subset of **OpenQASM 2.0**, generates an Abstract Syntax Tree (AST), lowers it into the **MLIR** framework (creating a custom `quantum` dialect), and performs circuit optimization passes (e.g., auto-inverse gate cancellation, rotation fusion). Finally, it re-emits optimized OpenQASM code and verifies semantic equivalence via simulation using Qiskit.

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

#### Create the build directory
```bash
mkdir build
cd build
```

#### Configure the build with CMake (Release mode, X86 target, enable MLIR)
```bash
cmake -G Ninja ../llvm \
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

## 💻 Usage
*This section will be updated as the compiler development progresses*

```bash
# Example usage (planned)
./bin/qcc input.qasm --optimize -emit=qasm -o output.qasm
```

## 📋 Project Roadmap
The development of this compiler is divided into the following milestones:
- [ ] **Task 1:** Parser and AST generation from OpenQASM subset.
- [ ] **Task 2:** Custom `quantum` dialect definition in MLIR (ODS/TableGen).
- [ ] **Task 3:** MLIRGen (AST visitor to SSA-style Intermediate Representation).
- [ ] **Task 4:** Pass 1 - Auto-inverse gate cancellation (e.g., H-H).
- [ ] **Task 5:** Pass 2 - Rotation fusion and parameter cleanup.
- [ ] **Task 6:** Backend - Re-emission of optimized OpenQASM code.
- [ ] **Task 7:** End-to-end equivalence verification via Qiskit Aer simulation.