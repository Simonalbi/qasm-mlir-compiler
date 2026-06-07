#ifndef FRONTEND_PARSER_H
#define FRONTEND_PARSER_H

#include "Lexer.h"
#include "AST.h"

#include <memory>

namespace quantum {

    /**
     * @brief Recursive Descent Parser for a subset of OpenQASM 2.0.
     *
     * Supported Grammar (EBNF):
     * Program      ::= VersionDecl? Statement* EOF
     * VersionDecl  ::= "OPENQASM" "2.0" ";"
     * Statement    ::= IncludeDecl | QRegDecl | CRegDecl | GateCall | Measure
     * IncludeDecl  ::= "include" StringLiteral ";"
     * QRegDecl     ::= "qreg" Identifier "[" Integer "]" ";"
     * CRegDecl     ::= "creg" Identifier "[" Integer "]" ";"
     * Measure      ::= "measure" Argument "->" Argument ";"
     * GateCall     ::= GateName ( "(" Expression ")" )? ArgumentList ";"
     * GateName     ::= "h" | "x" | "y" | "z" | "s" | "t" | "rx" | "ry" | "rz" | "cx"
     * ArgumentList ::= Argument ( "," Argument )*
     * Argument     ::= Identifier "[" Integer "]"
     * Expression   ::= "-"? PrimaryExpr ( BinaryOp PrimaryExpr )?
     * PrimaryExpr  ::= Float | Integer | "pi"
     * BinaryOp     ::= "+" | "-" | "*" | "/"
     */
    class Parser {
        Lexer &lex;
        int CurTok;
        bool HasError = false;

        /**
         * @brief Advances to the next token from the lexer.
         */
        int getNextToken() {
            return CurTok = lex.getToken();
        }

        // Error Handling Helpers
        std::unique_ptr<ExpressionAST> LogErrorExpr(const char *Str);
        std::unique_ptr<StatementAST> LogErrorStmt(const char *Str);

        /**
         * @brief Parses a primary expression (a single number or 'pi').
         */
        std::unique_ptr<ExpressionAST> parsePrimaryExpr();
        
        /**
         * @brief Parses an expression (e.g., `pi`, `1.57`, `pi/2`).
         */
        std::unique_ptr<ExpressionAST> parseExpression();

        /**
         * @brief Parses a target register entry like `q[0]`.
         *
         * @return A pair containing the register name and index.
         */
        std::pair<std::string, int> parseTarget();

        /**
         * @brief Parses `OPENQASM 2.0;`
         */
        std::unique_ptr<StatementAST> parseOpenQASMVersion();

        /**
         * @brief Parses `include "filename";`
         */
        std::unique_ptr<StatementAST> parseInclude();

        /**
         * @brief Parses `qreg q[2];` or `creg c[2];`
         */
        std::unique_ptr<StatementAST> parseRegisterDecl();

        /**
         * @brief Parses a gate application (e.g., `cx q[0], q[1];` or `rx(pi) q[0];`)
         */
        std::unique_ptr<StatementAST> parseGate();

        /**
         * @brief Parses a measurement (e.g., `measure q[0] -> c[0];`)
         */
        std::unique_ptr<StatementAST> parseMeasure();

        /**
         * @brief Determines the type of statement and routes to the correct parser.
         */
        std::unique_ptr<StatementAST> parseStatement();

    public:
        Parser(Lexer &lexer) : lex(lexer) {
            getNextToken(); // Initialize the first token
        }

        /**
         * @brief Parses the entire file and builds the AST.
         */
        std::unique_ptr<ProgramAST> parseProgram();

        /**
         * @brief Returns true if a syntax error was encountered during parsing.
         */
        bool hasError() const { return HasError; }
    };

}
#endif