#ifndef QUANTUM_PASSES_H
#define QUANTUM_PASSES_H

#include "mlir/Pass/Pass.h"
#include <memory>

namespace mlir {
    namespace quantum {

        // Creates a pass that verifies the No-Cloning theorem (each qubit must be used exactly once)
        std::unique_ptr<Pass> createNoCloningVerifierPass();

        // Registers all quantum dialect passes
        void registerQuantumPasses();
    }
}

#endif
