#include "Dialect/Passes.h"
#include "Dialect/QuantumDialect.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"

namespace mlir {
  namespace quantum {

    struct NoCloningVerifierPass : public PassWrapper<NoCloningVerifierPass, OperationPass<func::FuncOp>> {
      MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(NoCloningVerifierPass)

      StringRef getArgument() const final { return "quantum-verify-no-cloning"; }
      StringRef getDescription() const final { return "Verifies that all qubit values are used exactly once (No-Cloning Theorem)."; }

      void runOnOperation() override {
        func::FuncOp func = getOperation();
        bool failed = false;

        // Check all blocks
        for (Block &block : func.getBlocks()) {
          for (BlockArgument arg : block.getArguments()) {
            if (isa<QubitType>(arg.getType())) {
              if (!arg.hasOneUse()) {
                func.emitError("No-Cloning theorem violated: a block argument qubit must be used exactly once.");
                failed = true;
              }
            }
          }
          
          for (Operation &op : block) {
            for (Value result : op.getResults()) {
              if (isa<QubitType>(result.getType())) {
                if (!result.hasOneUse()) {
                  op.emitError("No-Cloning theorem violated: a qubit value must be used exactly once.");
                  failed = true;
                }
              }
            }
          }
        }
        
        if (failed) {
          signalPassFailure();
        }
      }
    };

    std::unique_ptr<Pass> createNoCloningVerifierPass() {
      return std::make_unique<NoCloningVerifierPass>();
    }

    void registerQuantumPasses() {
      PassRegistration<NoCloningVerifierPass>();
    }
  }
}
