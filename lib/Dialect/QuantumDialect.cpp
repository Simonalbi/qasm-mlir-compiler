#include "Dialect/QuantumDialect.h"
#include "llvm/ADT/TypeSwitch.h"
#include "mlir/IR/Builders.h"

using namespace mlir;
using namespace mlir::quantum;

//===----------------------------------------------------------------------===//
// Quantum dialect initialization
//===----------------------------------------------------------------------===//

void QuantumDialect::initialize() {
  addTypes<
#define GET_TYPEDEF_LIST
#include "QuantumTypes.cpp.inc"
      >();
      
  addOperations<
#define GET_OP_LIST
#include "QuantumOps.cpp.inc"
      >();
}

//===----------------------------------------------------------------------===//
// Quantum dialect auto-generated definitions
//===----------------------------------------------------------------------===//

#include "QuantumDialect.cpp.inc"

#define GET_TYPEDEF_CLASSES
#include "QuantumTypes.cpp.inc"

#define GET_OP_CLASSES
#include "QuantumOps.cpp.inc"
