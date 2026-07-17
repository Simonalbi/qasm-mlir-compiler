#include "Dialect/Passes.h"
#include "Dialect/QuantumDialect.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

using namespace mlir;
using namespace mlir::quantum;

namespace {

    template <typename UnaryOpT>
    struct CancelUnaryInversePattern : public OpRewritePattern<UnaryOpT> {
        using OpRewritePattern<UnaryOpT>::OpRewritePattern;

        LogicalResult matchAndRewrite(UnaryOpT op, PatternRewriter &rewriter) const override {
            // Check if the previous operation, if exists, is of the same type
            auto prevOp = op.getQIn().template getDefiningOp<UnaryOpT>();
            if (!prevOp) {
                return failure();
            }

            // Replace the current operation with the input of the previous operation, effectively canceling both
            rewriter.replaceOp(op, prevOp.getQIn());

            return success();
        }
    };

    struct CancelCXInversePattern : public OpRewritePattern<CXGateOp> {
        using OpRewritePattern<CXGateOp>::OpRewritePattern;

        LogicalResult matchAndRewrite(CXGateOp op, PatternRewriter &rewriter) const override {
            // Check if the previous operation, if exists, is a CXGateOp with the same control and target qubits
            auto prevCtrlOp = op.getCtrlIn().getDefiningOp<CXGateOp>();
            auto prevTargetOp = op.getTargetIn().getDefiningOp<CXGateOp>();

            if (!prevCtrlOp || prevCtrlOp != prevTargetOp) {
                return failure();
            }

            // Check if the control and target qubits match
            if (op.getCtrlIn() != prevCtrlOp.getCtrlOut() || 
                op.getTargetIn() != prevCtrlOp.getTargetOut()) {
                return failure();
            }

            // Replace the current operation with the inputs of the previous operation, effectively canceling both
            rewriter.replaceOp(op, {prevCtrlOp.getCtrlIn(), prevCtrlOp.getTargetIn()});

            return success();
        }
    };

    struct CancelInversePass : public PassWrapper<CancelInversePass, OperationPass<func::FuncOp>> {
        MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(CancelInversePass)

        StringRef getArgument() const final { return "quantum-cancel-inverse"; }
        StringRef getDescription() const final { return "Cancels consecutive auto-inverse quantum gates"; }

        void runOnOperation() override {
            RewritePatternSet patterns(&getContext());
            patterns.add<
                CancelUnaryInversePattern<HGateOp>,
                CancelUnaryInversePattern<XGateOp>,
                CancelUnaryInversePattern<YGateOp>,
                CancelUnaryInversePattern<ZGateOp>,
                CancelCXInversePattern
            >(&getContext());

            // Apply the patterns greedily to the function operation, MLIR will perform automatic DCE
            if (failed(applyPatternsGreedily(getOperation(), std::move(patterns)))) {
                signalPassFailure();
            }
        }
    };
}

namespace mlir {
    namespace quantum {
        std::unique_ptr<Pass> createCancelInversePass() {
            return std::make_unique<CancelInversePass>();
        }
        
        void registerCancelInversePass() {
            PassRegistration<CancelInversePass>();
        }
    }
}
