#ifndef FRONTEND_AST_H
#define FRONTEND_AST_H

#include <string>
#include <vector>
#include <memory>
#include "llvm/Support/Casting.h"

namespace quantum {

    enum class ASTNodeKind {
        BinaryOp,
        FloatExpr,
        PiExpr,
        RegisterDecl,
        Gate,
        Measure,
        VersionDecl,
        IncludeDecl,
        Program
    };

    /**
     * @brief Base class for all Abstract Syntax Tree (AST) nodes.
     *
     * Provides a common interface for all nodes in the AST.
     */
    class ASTNode {
        private:
            ASTNodeKind Kind;

        public:
            ASTNode(ASTNodeKind Kind) : Kind(Kind) {}
            virtual ~ASTNode() = default;
            ASTNodeKind getKind() const { return Kind; }
            virtual void dump(int indent = 0) const = 0;
    };

    /**
     * @brief Base class for all expression nodes.
     *
     * Expressions represent values that do not stand alone as execution steps,
     * such as parameters provided to parameterized quantum gates.
     */
    class ExpressionAST : public ASTNode {
        public:
            ExpressionAST(ASTNodeKind Kind) : ASTNode(Kind) {}

            static bool classof(const ASTNode *c) {
                return c->getKind() == ASTNodeKind::BinaryOp ||
                       c->getKind() == ASTNodeKind::FloatExpr ||
                       c->getKind() == ASTNodeKind::PiExpr;
            }
    };

    /**
     * @brief Represents a binary operation (e.g., +, -, *, /) between two expressions.
     */
    class BinaryOpAST : public ExpressionAST {
        private:
            char Op;
            std::unique_ptr<ExpressionAST> LHS;
            std::unique_ptr<ExpressionAST> RHS;
        public:
            BinaryOpAST(char Op, std::unique_ptr<ExpressionAST> LHS, std::unique_ptr<ExpressionAST> RHS) 
                : ExpressionAST(ASTNodeKind::BinaryOp), Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
            
            char getOp() const { return Op; }
            ExpressionAST* getLHS() const { return LHS.get(); }
            ExpressionAST* getRHS() const { return RHS.get(); }
            
            void dump(int indent = 0) const override;
            static bool classof(const ASTNode *c) { return c->getKind() == ASTNodeKind::BinaryOp; }
    };

    /**
     * @brief Base class for all statement nodes.
     *
     * Statements represent standalone actions or declarations within the program,
     * such as register declarations, includes, or gate applications.
     */
    class StatementAST : public ASTNode {
        public:
            StatementAST(ASTNodeKind Kind) : ASTNode(Kind) {}

            static bool classof(const ASTNode *c) {
                return c->getKind() == ASTNodeKind::RegisterDecl ||
                       c->getKind() == ASTNodeKind::Gate ||
                       c->getKind() == ASTNodeKind::Measure ||
                       c->getKind() == ASTNodeKind::VersionDecl ||
                       c->getKind() == ASTNodeKind::IncludeDecl;
            }
    };

    /**
     * @brief Represents a floating-point literal expression.
     */
    class FloatExprAST : public ExpressionAST {
        private:
            double Val;
        public:
            FloatExprAST(double Val) : ExpressionAST(ASTNodeKind::FloatExpr), Val(Val) {}
            
            double getVal() const { return Val; }
            
            void dump(int indent = 0) const override;
            static bool classof(const ASTNode *c) { return c->getKind() == ASTNodeKind::FloatExpr; }
    };

    /**
     * @brief Represents the mathematical constant 'pi' expression.
     */
    class PiExpressionAST : public ExpressionAST {
        public:
            PiExpressionAST() : ExpressionAST(ASTNodeKind::PiExpr) {}

            void dump(int indent = 0) const override;
            static bool classof(const ASTNode *c) { return c->getKind() == ASTNodeKind::PiExpr; }
    };

    /**
     * @brief Represents a quantum (qreg) or classical (creg) register declaration.
     */
    class RegisterDeclarationAST : public StatementAST {
        private:
            bool IsQuantum;
            std::string Name;
            int Size;
        public:
            RegisterDeclarationAST(bool IsQuantum, const std::string &Name, int Size) 
                : StatementAST(ASTNodeKind::RegisterDecl), IsQuantum(IsQuantum), Name(Name), Size(Size) {}
            
            bool isQuantum() const { return IsQuantum; }
            const std::string& getName() const { return Name; }
            int getSize() const { return Size; }
            
            void dump(int indent = 0) const override;
            static bool classof(const ASTNode *c) { return c->getKind() == ASTNodeKind::RegisterDecl; }
    };

    /**
     * @brief Represents the application of a quantum gate.
     *
     * Captures the gate identifier, any optional mathematical parameters,
     * and the target qubits it operates upon.
     */
    class GateAST : public StatementAST {
        private:
            std::string Name;
            std::vector<std::unique_ptr<ExpressionAST>> Params;
            std::vector<std::pair<std::string, int>> Targets;
        public:
            GateAST(const std::string &Name,
                    std::vector<std::unique_ptr<ExpressionAST>> Params,
                    std::vector<std::pair<std::string, int>> Targets
            ) : StatementAST(ASTNodeKind::Gate), Name(Name), Params(std::move(Params)), Targets(std::move(Targets)) {}
            
            const std::string& getName() const { return Name; }
            const std::vector<std::unique_ptr<ExpressionAST>>& getParams() const { return Params; }
            const std::vector<std::pair<std::string, int>>& getTargets() const { return Targets; }
            
            void dump(int indent = 0) const override;
            static bool classof(const ASTNode *c) { return c->getKind() == ASTNodeKind::Gate; }
    };

    /**
     * @brief Represents a measurement operation (e.g., `measure q[0] -> c[0];`)
     */
    class MeasureAST : public StatementAST {
        private:
            std::pair<std::string, int> Qubit;
            std::pair<std::string, int> ClassicalBit;
        public:
            MeasureAST(std::pair<std::string, int> Qubit, std::pair<std::string, int> ClassicalBit) 
                : StatementAST(ASTNodeKind::Measure), Qubit(Qubit), ClassicalBit(ClassicalBit) {}
            
            const std::pair<std::string, int>& getQubit() const { return Qubit; }
            const std::pair<std::string, int>& getClassicalBit() const { return ClassicalBit; }
            
            void dump(int indent = 0) const override;
            static bool classof(const ASTNode *c) { return c->getKind() == ASTNodeKind::Measure; }
    };

    /**
     * @brief Represents the OpenQASM version declaration (e.g. `OPENQASM 2.0;`)
     */
    class VersionDeclarationAST : public StatementAST {
        private:
            double Version;
        public:
            VersionDeclarationAST(double Version) : StatementAST(ASTNodeKind::VersionDecl), Version(Version) {}
            
            double getVersion() const { return Version; }
            
            void dump(int indent = 0) const override;
            static bool classof(const ASTNode *c) { return c->getKind() == ASTNodeKind::VersionDecl; }
    };

    /**
     * @brief Represents an include directive statement (e.g. `include "qelib1.inc";`)
     */
    class IncludeDeclarationAST : public StatementAST {
        private:
            std::string FileName;
        public:
            IncludeDeclarationAST(const std::string &FileName) : StatementAST(ASTNodeKind::IncludeDecl), FileName(FileName) {}
            
            const std::string& getFileName() const { return FileName; }
            
            void dump(int indent = 0) const override;
            static bool classof(const ASTNode *c) { return c->getKind() == ASTNodeKind::IncludeDecl; }
    };

    /**
     * @brief The root node of the AST, representing the entire OpenQASM program.
     */
    class ProgramAST : public ASTNode {
        private:
            std::vector<std::unique_ptr<StatementAST>> Statements;
        public:
            ProgramAST() : ASTNode(ASTNodeKind::Program) {}
            
            const std::vector<std::unique_ptr<StatementAST>>& getStatements() const { return Statements; }
            
            void addStatement(std::unique_ptr<StatementAST> stmt) {
                Statements.push_back(std::move(stmt));
            }
            
            void dump(int indent = 0) const override;
            static bool classof(const ASTNode *c) { return c->getKind() == ASTNodeKind::Program; }
    };
}
#endif