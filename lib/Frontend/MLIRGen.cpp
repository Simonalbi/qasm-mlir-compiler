#include "Frontend/MLIRGen.h"
#include "Frontend/AST.h"
#include "Dialect/QuantumDialect.h"

#include "mlir/IR/Attributes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "llvm/Support/Casting.h"

#include <cmath>
#include <map>
#include <vector>

namespace {

  /**
  * @brief Implementation class for the MLIR Generator Visitor.
  * 
  * Traverses the AST and constructs the equivalent MLIR Intermediate Representation.
  * Uses `qubitMap` to enforce the Single Static Assignment (SSA) form required by
  * the quantum dialect, naturally satisfying the No-Cloning theorem.
  */
  class MLIRGenImpl {
    public:
      MLIRGenImpl(mlir::MLIRContext &context) : builder(&context), context(&context) {}

      /**
      * @brief Main entry point to generate MLIR for the entire AST.
      *
      * This method orchestrates the complete lowering of a Quantum AST into MLIR operations.
      * It operates in 4 distinct phases to correctly model the quantum architecture.
      *
      * @details
      * ### Phase 1: Function Signature Setup
      * Translates global OpenQASM register declarations into MLIR function parameters.
      * For example, given `qreg q[2];` and `qreg p[1];` in the AST, it computes a signature:
      *
      *     func.func @circuit(%arg0: !quantum.qreg<2>, %arg1: !quantum.qreg<1>) -> (!quantum.qubit, !quantum.qubit, !quantum.qubit)
      *
      * ### Phase 2: Qubit Extraction and Tracking Initialization
      * Quantum gates operate on individual qubits, not entire arrays. This phase 
      * iterates over the function arguments and extracts each qubit, registering it 
      * into the `qubitMap` to establish the initial SSA state mapping.
      *     
      *     func.func @circuit(%arg0: !quantum.qreg<2>, %arg1: !quantum.qreg<1>) -> (!quantum.qubit, !quantum.qubit, !quantum.qubit) {
      *         %0 = quantum.extract %arg0[0] : !quantum.qreg<2> -> !quantum.qubit
      *         %1 = quantum.extract %arg0[1] : !quantum.qreg<2> -> !quantum.qubit
      *         %2 = quantum.extract %arg1[0] : !quantum.qreg<1> -> !quantum.qubit
      *     }
      *
      * ### Phase 3: AST Traversal and Operations Emission
      * Iterates through the AST statements (e.g., gates, measurements) and lowers them
      * into their MLIR counterparts using the SSA values tracked in `qubitMap`.
      * For example, given `h q[0];` and `cx q[0], p[0];`:
      *
      *     func.func @circuit(%arg0: !quantum.qreg<2>, %arg1: !quantum.qreg<1>) -> (!quantum.qubit, !quantum.qubit, !quantum.qubit) {
      *         %0 = quantum.extract %arg0[0] : !quantum.qreg<2> -> !quantum.qubit
      *         %1 = quantum.extract %arg0[1] : !quantum.qreg<2> -> !quantum.qubit
      *         %2 = quantum.extract %arg1[0] : !quantum.qreg<1> -> !quantum.qubit
      *
      *         %3 = quantum.h %0 : !quantum.qubit -> !quantum.qubit
      *         %4, %5 = quantum.cx %3, %2 : !quantum.qubit, !quantum.qubit -> !quantum.qubit, !quantum.qubit
      *     }
      *
      * ### Phase 4: Termination
      * Gathers the final state of all allocated qubits in their chronological order
      * and returns them to prevent the optimizer from eliminating unmeasured gates as dead code.
      *
      *     func.func @circuit(%arg0: !quantum.qreg<2>, %arg1: !quantum.qreg<1>) -> (!quantum.qubit, !quantum.qubit, !quantum.qubit) {
      *         %0 = quantum.extract %arg0[0] : !quantum.qreg<2> -> !quantum.qubit
      *         %1 = quantum.extract %arg0[1] : !quantum.qreg<2> -> !quantum.qubit
      *         %2 = quantum.extract %arg1[0] : !quantum.qreg<1> -> !quantum.qubit
      *
      *         %3 = quantum.h %0 : !quantum.qubit -> !quantum.qubit
      *         %4, %5 = quantum.cx %3, %2 : !quantum.qubit, !quantum.qubit -> !quantum.qubit, !quantum.qubit
      *
      *         return %4, %1, %5 : !quantum.qubit, !quantum.qubit, !quantum.qubit
      *     }
      *
      * @param ast The root node of the Quantum AST to be translated.
      *
      * @return A fully populated MLIR ModuleOp containing the lowered quantum circuit.
      */
      mlir::ModuleOp mlirGen(::quantum::ProgramAST &ast) {
        theModule = mlir::ModuleOp::create(builder.getUnknownLoc());

        // ============= Function Signature Setup =============

        // Prepare the function signature: inputs are quantum registers, outputs are individual qubits
        std::vector<mlir::Type> argTypes;
        std::vector<mlir::Type> resultTypes;
        std::vector<::quantum::RegisterDeclarationAST*> qregs;
        
        // Iterate through the AST to collect all quantum register declarations and prepare the function signature
        for (auto &stmt_ptr : ast.getStatements()) {
          if (auto *qreg = llvm::dyn_cast<::quantum::RegisterDeclarationAST>(stmt_ptr.get())) {
            if (qreg->isQuantum()) {
              // Store the quantum register for later extraction and tracking
              qregs.push_back(qreg);
              argTypes.push_back(mlir::quantum::QregType::get(context, qreg->getSize()));

              // For each qubit in the register, we will return its final state to keep it alive
              for (int i = 0; i < qreg->getSize(); ++i) {
                resultTypes.push_back(mlir::quantum::QubitType::get(context));
              }
            }
          }
        }

        // Create the main MLIR function that will contain the quantum circuit operations
        auto funcType = builder.getFunctionType(argTypes, resultTypes);
        auto funcOp = mlir::func::FuncOp::create(builder.getUnknownLoc(), "circuit", funcType);

        auto *entryBlock = funcOp.addEntryBlock();
        builder.setInsertionPointToStart(entryBlock);

        // ============= Qubit Extraction and Tracking Initialization =============

        auto qubitType = mlir::quantum::QubitType::get(context);
        int argIdx = 0;

        // Iterate over each declared quantum register (e.g. qreg q[3])
        for (auto *qreg : qregs) {
          // Retrieve the function parameter corresponding to this whole register array
          mlir::Value qregVal = funcOp.getArgument(argIdx++);
          
          // Iterate over each individual qubit inside the register
          for (int i = 0; i < qreg->getSize(); ++i) {
            // Extracts the single i-th qubit from the register
            auto extractOp = mlir::quantum::ExtractQubitOp::create(
              builder,
              builder.getUnknownLoc(), 
              qubitType, 
              qregVal, 
              builder.getI64IntegerAttr(i)
            );
            
            // Map the QASM identifier (e.g. "q", 0) to the purely SSA MLIR value extracted (e.g. %0)
            qubitMap[{qreg->getName(), i}] = extractOp.getResult();
            
            // Append to a chronological list to remember the exact order quants were allocated
            qubitOrder.push_back({qreg->getName(), i});
          }
        }

        // ============= AST Traversal and Operations Emission =============

        // Iterate through the AST statements and generate MLIR for each gate or measurement operation
        for (auto &stmt_ptr : ast.getStatements()) {
          if (mlir::failed(mlirGen(*stmt_ptr))) {
            theModule.erase();
            return nullptr;
          }
        }

        // ============= Termination =============

        // Collect the final SSA values of all qubits in the order they were originally allocated
        std::vector<mlir::Value> retValues;
        for (const auto &q : qubitOrder) {
          retValues.push_back(qubitMap[q]);
        }
        mlir::func::ReturnOp::create(builder, builder.getUnknownLoc(), retValues);

        // Add the completed function to the module and return it
        theModule.push_back(funcOp);
        return theModule;
      }

    private:
      mlir::OpBuilder builder;      ///< MLIR builder used to emit operations
      mlir::MLIRContext *context;   ///< MLIR context
      mlir::ModuleOp theModule;     ///< The top-level MLIR module being built
      
      /// Maps a QASM qubit identifier (e.g. "q", 0) to its current MLIR SSA Value (e.g. %0)
      std::map<std::pair<std::string, int>, mlir::Value> qubitMap;

      /// Keeps track of the original allocation order for deterministic returns
      std::vector<std::pair<std::string, int>> qubitOrder;

      /**
      * @brief Dispatcher for AST Nodes.
      *
      * Attempt to dynamically cast generic AST nodes into their concrete
      * types (e.g., GateAST, MeasureAST) and routing them to the specialized
      * mlirGen functions. Nodes that do not emit MLIR are safely ignored.
      *
      * @param node The AST node to be processed.
      *
      * @return mlir::success() if the node was successfully lowered or ignored, mlir::failure() otherwise.
      */
      mlir::LogicalResult mlirGen(::quantum::ASTNode &node) {
        if (auto *gate = llvm::dyn_cast<::quantum::GateAST>(&node)) {
          // Dispatch to the specialized gate lowering function
          return mlirGen(*gate);
        } else if (auto *meas = llvm::dyn_cast<::quantum::MeasureAST>(&node)) {
          // Dispatch to the specialized measurement lowering function
          return mlirGen(*meas);
        }

        // Ignore statements that do not emit MLIR (e.g. IncludeDecl, VersionDecl, CRegDecl)
        return mlir::success(); 
      }

      /**
      * @brief Generates MLIR for a Quantum Gate.
      *
      * This function lowers a gate AST node into the corresponding MLIR operation.
      *
      * @param gate The gate AST node to be lowered.
      *
      * @return mlir::success() if the gate was successfully lowered, mlir::failure() otherwise.
      */
      mlir::LogicalResult mlirGen(::quantum::GateAST &gate) {
        auto loc = builder.getUnknownLoc();

        // Retrieve the MLIR type for qubits, gate's name and targets
        auto qubitType = mlir::quantum::QubitType::get(context);
        auto gateName = gate.getName();
        auto targets = gate.getTargets();
        
        // ============= Binary Gate (CNOT) =============

        if (gateName == "cx") {
          if (targets.size() != 2) return mlir::failure();
          
          // Consume the current inputs
          auto ctrl = qubitMap[targets[0]];
          auto targ = qubitMap[targets[1]];
          
          // Emit the MLIR operation
          auto cx = mlir::quantum::CXGateOp::create(builder, loc, qubitType, qubitType, ctrl, targ);
          
          // Update the tracking map with the new output values (SSA update)
          qubitMap[targets[0]] = cx.getResult(0);
          qubitMap[targets[1]] = cx.getResult(1);
          return mlir::success();
        }
        
        // ============= Unary Gates =============

        // For unary gates, we expect exactly one target qubit
        if (targets.size() != 1) return mlir::failure();
        
        // Consume the input qubit
        auto q_in = qubitMap[targets[0]];
        mlir::Value q_out;

        // Create the appropriate un-parameterized gate
        if (gateName == "h")      q_out = mlir::quantum::HGateOp::create(builder, loc, qubitType, q_in).getResult();
        else if (gateName == "x") q_out = mlir::quantum::XGateOp::create(builder, loc, qubitType, q_in).getResult();
        else if (gateName == "y") q_out = mlir::quantum::YGateOp::create(builder, loc, qubitType, q_in).getResult();
        else if (gateName == "z") q_out = mlir::quantum::ZGateOp::create(builder, loc, qubitType, q_in).getResult();
        else if (gateName == "s") q_out = mlir::quantum::SGateOp::create(builder, loc, qubitType, q_in).getResult();
        else if (gateName == "t") q_out = mlir::quantum::TGateOp::create(builder, loc, qubitType, q_in).getResult();
        
        // Create the appropriate parameterized gate
        else if (gateName == "rx" || gateName == "ry" || gateName == "rz") {
          auto& params = gate.getParams();
          if (params.empty()) {
            return mlir::failure();
          }
          
          // Evaluate the angle expression to a double value
          double theta = evalExpr(params[0].get());
          auto attr = builder.getF64FloatAttr(theta);
          
          if (gateName == "rx")       q_out = mlir::quantum::RxGateOp::create(builder, loc, qubitType, attr, q_in).getResult();
          else if (gateName == "ry")  q_out = mlir::quantum::RyGateOp::create(builder, loc, qubitType, attr, q_in).getResult();
          else if (gateName == "rz")  q_out = mlir::quantum::RzGateOp::create(builder, loc, qubitType, attr, q_in).getResult();
        } else {
          // Unknown gate, skip safely
          return mlir::success();
        }

        // Update the tracking map with the newly generated qubit value
        if (q_out) {
          qubitMap[targets[0]] = q_out;
        }

        return mlir::success();
      }

      /**
      * @brief Generates MLIR for a Measurement Operation.
      *
      * This function lowers a measurement AST node into the corresponding MLIR operation.
      *
      * @param meas The measurement AST node to be lowered.
      * 
      * @return mlir::success() if the measurement was successfully lowered, mlir::failure() otherwise.
      */
      mlir::LogicalResult mlirGen(::quantum::MeasureAST &meas) {
        auto loc = builder.getUnknownLoc();

        // Tell the builder the types that will be produced by the measurement operation: a qubit and a classical bit
        auto qubitType = mlir::quantum::QubitType::get(context);
        auto i1Type = builder.getI1Type();

        // Get the current SSA value of the qubit to be measured from the tracking map
        auto q_in = qubitMap[meas.getQubit()];

        // Emit the MLIR measurement operation, which returns both the updated qubit and a classical bit
        auto op = mlir::quantum::MeasureOp::create(builder, loc, qubitType, i1Type, q_in);
        
        // The measure op returns an updated qubit (to maintain the SSA chain) and a classical bit
        qubitMap[meas.getQubit()] = op.getResult(0);
        return mlir::success();
      }

      /**
      * @brief Recursively evaluates an AST mathematical expression into a scalar double.
      *
      * This function handles basic arithmetic operations and constants, including
      * floating-point literals and the mathematical constant Pi. It is used to compute
      * the angles for parameterized quantum gates (e.g., rx, ry, rz).
      *
      * @param expr The root of the expression AST to evaluate.
      *
      * @return The computed double value of the expression.
      */
      double evalExpr(::quantum::ExpressionAST *expr) {
        if (auto *f = llvm::dyn_cast<::quantum::FloatExprAST>(expr)) {
          return f->getVal();
        } else if (llvm::isa<::quantum::PiExpressionAST>(expr)) {
          return M_PI;
        } else if (auto *b = llvm::dyn_cast<::quantum::BinaryOpAST>(expr)) {
          double lhs = evalExpr(b->getLHS());
          double rhs = evalExpr(b->getRHS());

          switch (b->getOp()) {
            case '+': return lhs + rhs;
            case '-': return lhs - rhs;
            case '*': return lhs * rhs;
            case '/': return lhs / rhs;
          }
        }

        // If the expression type is unrecognized, return 0.0 as a safe default
        return 0.0;
      }
  };
}

namespace quantum {
  mlir::OwningOpRef<mlir::ModuleOp> mlirGen(mlir::MLIRContext &context, ProgramAST &ast) {
    return MLIRGenImpl(context).mlirGen(ast);
  }
}
