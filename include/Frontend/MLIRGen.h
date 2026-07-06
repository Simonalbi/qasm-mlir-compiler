#ifndef FRONTEND_MLIRGEN_H
#define FRONTEND_MLIRGEN_H

namespace mlir {
    class MLIRContext;
    class ModuleOp;
    template <typename OpTy> class OwningOpRef;
}

namespace quantum {
    class ProgramAST;

    /**
    * @brief Generates MLIR Intermediate Representation from the OpenQASM AST.
    * 
    * This function initiates the traversal of the provided AST, constructing
    * equivalent MLIR operations using the custom `quantum` dialect. It applies
    * strict SSA value semantics to enforce the No-Cloning Theorem.
    * 
    * @param context The active MLIR context where dialects are registered.
    * @param ast The root node of the parsed Abstract Syntax Tree.
    *
    * @return An owning reference to the generated MLIR ModuleOp, or null on failure.
    */
    mlir::OwningOpRef<mlir::ModuleOp> mlirGen(mlir::MLIRContext &context, ProgramAST &ast);
}

#endif
