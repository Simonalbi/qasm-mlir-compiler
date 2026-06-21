#ifndef QUANTUM_DIALECT_H
#define QUANTUM_DIALECT_H

#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/DialectImplementation.h"
#include "mlir/Bytecode/BytecodeOpInterface.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"

// Include the auto-generated headers
#include "QuantumDialect.h.inc"

#define GET_TYPEDEF_CLASSES
#include "QuantumTypes.h.inc"

#define GET_OP_CLASSES
#include "QuantumOps.h.inc"

#endif // QUANTUM_DIALECT_H
