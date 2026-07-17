#ifndef QUANTUM_PASSES_H
#define QUANTUM_PASSES_H

#include "mlir/Pass/Pass.h"
#include <memory>

namespace mlir {
    namespace quantum {
        std::unique_ptr<Pass> createNoCloningVerifierPass();
        std::unique_ptr<Pass> createCancelInversePass();
        
        void registerNoCloningVerifierPass();
        void registerCancelInversePass();

        inline void registerQuantumPasses() {
            registerNoCloningVerifierPass();
            registerCancelInversePass();
        }
    }
}

#endif
