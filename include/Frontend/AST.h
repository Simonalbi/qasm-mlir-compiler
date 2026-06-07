#ifndef FRONTEND_AST_H
#define FRONTEND_AST_H

#include <string>
#include <vector>
#include <memory>

namespace quantum {

    /**
     * @brief Base class for all Abstract Syntax Tree (AST) nodes.
     *
     * Provides a common interface for all nodes in the AST.
     */
    class ASTNode {
        public:
            virtual ~ASTNode() = default;
            virtual void dump(int indent = 0) const = 0;
    };

    /**
     * @brief Base class for all expression nodes.
     *
     * Expressions represent values that do not stand alone as execution steps,
     * such as parameters provided to parameterized quantum gates.
     */
    class ExpressionAST : public ASTNode {};

    /**
     * @brief Represents a binary operation (e.g., +, -, *, /) between two expressions.
     */
    class BinaryOpAST : public ExpressionAST {
        char Op;
        std::unique_ptr<ExpressionAST> LHS;
        std::unique_ptr<ExpressionAST> RHS;

        public:
            BinaryOpAST(char Op, std::unique_ptr<ExpressionAST> LHS, std::unique_ptr<ExpressionAST> RHS) : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
            void dump(int indent = 0) const override;
    };

    /**
     * @brief Base class for all statement nodes.
     *
     * Statements represent standalone actions or declarations within the program,
     * such as register declarations, includes, or gate applications.
     */
    class StatementAST : public ASTNode {};

    /**
     * @brief Represents a floating-point literal expression.
     */
    class FloatExprAST : public ExpressionAST {
        double Val;

        public:
            FloatExprAST(double Val) : Val(Val) {}
            void dump(int indent = 0) const override;
    };

    /**
     * @brief Represents the mathematical constant 'pi' expression.
     */
    class PiExpressionAST : public ExpressionAST {
        public:
            void dump(int indent = 0) const override;
    };

    /**
     * @brief Represents a quantum (qreg) or classical (creg) register declaration.
     */
    class RegisterDeclarationAST : public StatementAST {
        bool IsQuantum;
        std::string Name;
        int Size;

        public:
            RegisterDeclarationAST(bool IsQuantum, const std::string &Name, int Size) : IsQuantum(IsQuantum), Name(Name), Size(Size) {}
            void dump(int indent = 0) const override;
    };

    /**
     * @brief Represents the application of a quantum gate.
     *
     * Captures the gate identifier, any optional mathematical parameters,
     * and the target qubits it operates upon.
     */
    class GateAST : public StatementAST {
        std::string Name;
        std::vector<std::unique_ptr<ExpressionAST>> Params;
        std::vector<std::pair<std::string, int>> Targets;

        public:
            GateAST(const std::string &Name,
                    std::vector<std::unique_ptr<ExpressionAST>> Params,
                    std::vector<std::pair<std::string, int>> Targets
            ) : Name(Name), Params(std::move(Params)), Targets(std::move(Targets)) {}
            void dump(int indent = 0) const override;
    };

    /**
     * @brief Represents a measurement operation (e.g., `measure q[0] -> c[0];`)
     */
    class MeasureAST : public StatementAST {
        std::pair<std::string, int> Qubit;
        std::pair<std::string, int> ClassicalBit;

        public:
            MeasureAST(std::pair<std::string, int> Qubit, std::pair<std::string, int> ClassicalBit) : Qubit(Qubit), ClassicalBit(ClassicalBit) {}
            void dump(int indent = 0) const override;
    };

    /**
     * @brief Represents the OpenQASM version declaration (e.g. `OPENQASM 2.0;`)
     */
    class VersionDeclarationAST : public StatementAST {
        double Version;

        public:
            VersionDeclarationAST(double Version) : Version(Version) {}
            void dump(int indent = 0) const override;
    };

    /**
     * @brief Represents an include directive statement (e.g. `include "qelib1.inc";`)
     */
    class IncludeDeclarationAST : public StatementAST {
        std::string FileName;

        public:
            IncludeDeclarationAST(const std::string &FileName) : FileName(FileName) {}
            void dump(int indent = 0) const override;
    };

    /**
     * @brief The root node of the AST, representing the entire OpenQASM program.
     */
    class ProgramAST : public ASTNode {
        std::vector<std::unique_ptr<StatementAST>> Statements;

        public:
            void addStatement(std::unique_ptr<StatementAST> stmt) {
                Statements.push_back(std::move(stmt));
            }
            void dump(int indent = 0) const override;
    };
}
#endif