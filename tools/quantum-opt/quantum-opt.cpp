#include "mlir/IR/Dialect.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"

#include "Dialect/QuantumDialect.h"
#include "Dialect/Passes.h"

int main(int argc, char **argv) {
  mlir::DialectRegistry registry;
  registry.insert<mlir::quantum::QuantumDialect, mlir::func::FuncDialect>();

  mlir::quantum::registerQuantumPasses();

  // Use the mlir-opt main driver
  return mlir::asMainReturnCode(
    mlir::MlirOptMain(argc, argv, "Quantum Optimizer Driver\n", registry)
  );
}
