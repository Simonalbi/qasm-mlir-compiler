#ifndef QUANTUM_TD
#define QUANTUM_TD

include "mlir/IR/OpBase.td"
include "mlir/IR/AttrTypeBase.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

//===----------------------------------------------------------------------===//
// Quantum Dialect Definition
//===----------------------------------------------------------------------===//

def Quantum_Dialect : Dialect {
  let name = "quantum";
  let summary = "A quantum dialect for a subset of OpenQASM 2.0";
  let description = [{
    This dialect provides operations and types to model quantum circuits, 
    designed as a compilation target for a subset of OpenQASM 2.0.
    
    It models qubits using strict value semantics (SSA-style), where each quantum gate 
    consumes an input qubit value and produces a new updated qubit value.
    This representation enables safe and direct pattern matching for optimization
    passes like auto-inverse cancellation (e.g., H(H(x)) -> x).

    The dialect supports types like `!quantum.qubit` and `!quantum.qreg<N>`, and a core 
    gate set including {h, x, y, z, s, t, rx, ry, rz, cx, measure}.
  }];
  
  // The C++ namespace that the dialect class definition resides in.
  let cppNamespace = "::mlir::quantum";
  
  // Automatically generate the logic for printing and parsing custom types
  let useDefaultTypePrinterParser = 1;
}

//===----------------------------------------------------------------------===//
// Quantum Types Definition
//===----------------------------------------------------------------------===//

// Base type definition for quantum types
class Quantum_Type<string name, string typeMnemonic, list<Trait> traits = []> : TypeDef<Quantum_Dialect, name, traits> {
  let mnemonic = typeMnemonic;
}

// !quantum.qubit
def QubitType : Quantum_Type<"Qubit", "qubit"> {
  let summary = "A single quantum bit";
  let description = [{
    Represents a single qubit passed through quantum gates.
  }];
}

// !quantum.qreg<N>
def QregType : Quantum_Type<"Qreg", "qreg"> {
  let summary = "A register of N quantum bits";
  let description = [{
    Represents an array of qubits of a fixed size.
    Example: `!quantum.qreg<5>` represents a 5-qubit register.
  }];
  
  // Parameter for the size of the quantum register
  let parameters = (ins "int":$size);
  
  // Custom assembly format to print the size parameter
  let assemblyFormat = "`<` $size `>`";
}

//===----------------------------------------------------------------------===//
// Quantum Operations Definition
//===----------------------------------------------------------------------===//

// Base class for all quantum operations
class Quantum_Op<string mnemonic, list<Trait> traits = []> :  Op<Quantum_Dialect, mnemonic, traits>;

// Unary gate base class (no parameters, 1 qubit in, 1 qubit out)
class Quantum_UnaryGate<string mnemonic, list<Trait> traits = [Pure]> : Quantum_Op<mnemonic, traits> {
  let arguments = (ins QubitType:$q_in);
  let results = (outs QubitType:$q_out);
  let assemblyFormat = "$q_in attr-dict `:` type($q_in) `->` type($q_out)";
}

// Unary parameterized gate base class (1 parameter, 1 qubit in, 1 qubit out)
class Quantum_UnaryParamGate<string mnemonic, list<Trait> traits = [Pure]> : Quantum_Op<mnemonic, traits> {
  let arguments = (ins F64Attr:$theta, QubitType:$q_in);
  let results = (outs QubitType:$q_out);
  let assemblyFormat = "$theta `,` $q_in attr-dict `:` type($q_in) `->` type($q_out)";
}

// Define the standard unary gates
def HGateOp : Quantum_UnaryGate<"h"> { let summary = "Hadamard gate"; }
def XGateOp : Quantum_UnaryGate<"x"> { let summary = "Pauli-X gate"; }
def YGateOp : Quantum_UnaryGate<"y"> { let summary = "Pauli-Y gate"; }
def ZGateOp : Quantum_UnaryGate<"z"> { let summary = "Pauli-Z gate"; }
def SGateOp : Quantum_UnaryGate<"s"> { let summary = "S (Phase) gate"; }
def TGateOp : Quantum_UnaryGate<"t"> { let summary = "T (pi/8) gate"; }

// Define the parameterized unary gates
def RxGateOp : Quantum_UnaryParamGate<"rx"> { let summary = "RX gate"; }
def RyGateOp : Quantum_UnaryParamGate<"ry"> { let summary = "RY gate"; }
def RzGateOp : Quantum_UnaryParamGate<"rz"> { let summary = "RZ gate"; }

// Binary gate (CNOT)
def CXGateOp : Quantum_Op<"cx", [Pure]> {
  let summary = "Controlled-NOT gate";
  let arguments = (ins QubitType:$ctrl_in, QubitType:$target_in);
  let results = (outs QubitType:$ctrl_out, QubitType:$target_out);
  let assemblyFormat = "$ctrl_in `,` $target_in attr-dict `:` type($ctrl_in) `,` type($target_in) `->` type($ctrl_out) `,` type($target_out)";
}

// Measure gate
def MeasureOp : Quantum_Op<"measure"> {
  let summary = "Measure qubit into a classical bit";
  let arguments = (ins QubitType:$q_in);
  // Returns updated qubit to maintain SSA chain + the classical bit result
  let results = (outs QubitType:$q_out, I1:$res);
  let assemblyFormat = "$q_in attr-dict `:` type($q_in) `->` type($q_out) `,` type($res)";
}

// Allocation/Extraction ops (to map OpenQASM arrays to SSA values)
def ExtractQubitOp : Quantum_Op<"extract", [Pure]> {
  let summary = "Extract a single qubit from a quantum register";
  let arguments = (ins QregType:$qreg, I64Attr:$index);
  let results = (outs QubitType:$q_out);
  let assemblyFormat = "$qreg `[` $index `]` attr-dict `:` type($qreg) `->` type($q_out)";
}

#endif
