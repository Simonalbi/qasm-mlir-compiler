#include "Frontend/Parser.h"

#include <utility>

namespace quantum {

    std::unique_ptr<ExpressionAST> Parser::LogErrorExpr(const std::string &Str) {
        HasError = true;
        fprintf(stderr, "Syntax Error at Line %d, Col %d: %s\n", lex.getLine(), lex.getCol(), Str.c_str());
        return nullptr;
    }

    std::unique_ptr<StatementAST> Parser::LogErrorStmt(const std::string &Str) {
        HasError = true;
        fprintf(stderr, "Syntax Error at Line %d, Col %d: %s\n", lex.getLine(), lex.getCol(), Str.c_str());
        return nullptr;
    }

    std::unique_ptr<ExpressionAST> Parser::parsePrimaryExpr() {
        if (CurTok == tok_pi) {
            // Consume 'pi'
            getNextToken();
            return std::make_unique<PiExpressionAST>();
        } else if (CurTok == tok_float || CurTok == tok_integer) {
            // Consume number
            double val = (CurTok == tok_float) ? lex.getFloatVal() : (double)lex.getNumVal();
            getNextToken();
            return std::make_unique<FloatExprAST>(val);
        }

        return LogErrorExpr("Expected expression (float or 'pi')");
    }

    std::unique_ptr<ExpressionAST> Parser::parseExpression() {
        // Consume '-'
        bool isNegative = false;
        if (CurTok == '-') {
            isNegative = true;
            getNextToken();
        }

        // Parse LHS
        auto LHS = parsePrimaryExpr();
        if (!LHS) return nullptr;

        // Handle unary minus by wrapping LHS in a BinaryOpAST (* -1.0)
        if (isNegative) {
            auto minusOne = std::make_unique<FloatExprAST>(-1.0);
            LHS = std::make_unique<BinaryOpAST>('*', std::move(minusOne), std::move(LHS));
        }

        // Check for binary operator
        if (CurTok == '+' || CurTok == '-' || CurTok == '*' || CurTok == '/') {
            // Consume operator
            char Op = CurTok;
            getNextToken();

            // Parse RHS
            auto RHS = parsePrimaryExpr();
            if (!RHS) return nullptr;

            // Return a BinaryOpAST node
            return std::make_unique<BinaryOpAST>(Op, std::move(LHS), std::move(RHS));
        }

        // If there was no operator (e.g. just 'pi'), simply return the LHS
        return LHS;
    }

    bool Parser::parseTarget(std::pair<std::string, int> &Target, TargetKind ExpectedKind, const std::string &Context) {
        if (CurTok != tok_identifier) {
            LogErrorStmt("Expected identifier for register name");
            return false;
        }

        // Consume identifier
        std::string regName = lex.getIdentifierStr();
        getNextToken();

        // Consume '['
        if (CurTok != '[') {
            LogErrorStmt("Expected '[' after register name");
            return false;
        }
        getNextToken();

        // Consume integer
        if (CurTok != tok_integer) {
            LogErrorStmt("Expected integer index inside '[' ']'");
            return false;
        }

        int index = lex.getNumVal();
        getNextToken();

        // Consume ']'
        if (CurTok != ']') {
            LogErrorStmt("Expected ']' after register index");
            return false;
        }
        getNextToken();

        // Verify that the register exists
        auto regIt = Registers.find(regName);
        if (regIt == Registers.end()) {
            LogErrorStmt("Register '" + regName + "' is not declared before use in " + Context);
            return false;
        }

        // Verify that the register kind matches the expected kind
        const auto &regInfo = regIt->second;
        if ((ExpectedKind == TargetKind::Quantum && !regInfo.IsQuantum) ||
            (ExpectedKind == TargetKind::Classical && regInfo.IsQuantum)) {
            LogErrorStmt("Register '" + regName + "' has the wrong kind for " + Context);
            return false;
        }

        // Verify that the index is within bounds
        if (index < 0 || index >= regInfo.Size) {
            LogErrorStmt("Index out of range for register '" + regName + "' in " + Context);
            return false;
        }

        Target = {regName, index};
        return true;
    }

    bool Parser::isSupportedGate(const std::string &GateName) const {
        return GateName == "h" || GateName == "x" || GateName == "y" || GateName == "z" ||
               GateName == "s" || GateName == "t" || GateName == "cx" ||
               GateName == "rx" || GateName == "ry" || GateName == "rz";
    }

    bool Parser::gateExpectsParameter(const std::string &GateName) const {
        return GateName == "rx" || GateName == "ry" || GateName == "rz";
    }

    size_t Parser::gateTargetCount(const std::string &GateName) const {
        return GateName == "cx" ? 2 : 1;
    }

    std::unique_ptr<StatementAST> Parser::parseOpenQASMVersion() {
        // Consume 'OPENQASM'
        getNextToken();
        
        // Consume version
        if (CurTok != tok_float) return LogErrorStmt("Expected version number (e.g., 2.0)");
        double version = lex.getFloatVal();
        getNextToken(); 

        // Consume ';'
        if (CurTok != ';') return LogErrorStmt("Expected ';' after OPENQASM declaration");
        getNextToken(); 

        return std::make_unique<VersionDeclarationAST>(version);
    }

    std::unique_ptr<StatementAST> Parser::parseInclude() {
        // Consume 'include'
        getNextToken(); 

        // Consume string
        if (CurTok != tok_string) return LogErrorStmt("Expected string literal after include");
        std::string filename = lex.getStringVal();
        getNextToken(); 

        // Consume ';'
        if (CurTok != ';') return LogErrorStmt("Expected ';' after include statement");
        getNextToken();

        return std::make_unique<IncludeDeclarationAST>(filename);
    }

    std::unique_ptr<StatementAST> Parser::parseRegisterDecl() {
        // Consume 'qreg' or 'creg'
        bool isQuantum = (CurTok == tok_qreg);
        getNextToken(); 
        
        // Consume name
        if (CurTok != tok_identifier) return LogErrorStmt("Expected register name");
        std::string name = lex.getIdentifierStr();
        getNextToken();

        // Consume '['
        if (CurTok != '[') return LogErrorStmt("Expected '['");
        getNextToken();

        // Consume integer
        if (CurTok != tok_integer) return LogErrorStmt("Expected register size (integer)");
        int size = lex.getNumVal();
        getNextToken(); 

        // Consume ']'
        if (CurTok != ']') return LogErrorStmt("Expected ']'");
        getNextToken(); 

        // Consume ';'
        if (CurTok != ';') return LogErrorStmt("Expected ';' after register declaration");
        getNextToken();

        Registers[name] = {isQuantum, size};

        return std::make_unique<RegisterDeclarationAST>(isQuantum, name, size);
    }

    std::unique_ptr<StatementAST> Parser::parseGate() {
        // Consume identifier
        std::string gateName = lex.getIdentifierStr();
        if (!isSupportedGate(gateName)) {
            return LogErrorStmt("Unsupported gate: '" + gateName + "'");
        }

        getNextToken();

        const bool expectsParameter = gateExpectsParameter(gateName);
        std::vector<std::unique_ptr<ExpressionAST>> params;

        // Parse single param
        if (CurTok == '(') {
            if (!expectsParameter) {
                return LogErrorStmt("Gate '" + gateName + "' does not take parameters");
            }

            // Consume '('
            getNextToken();

            // Parse expression
            if (auto expr = parseExpression()) {
                params.push_back(std::move(expr));
            } else {
                return nullptr;
            }

            if (CurTok != ')') return LogErrorStmt("Expected ')' after parameters");

            // Consume ')'
            getNextToken();
        } else if (expectsParameter) {
            return LogErrorStmt("Gate '" + gateName + "' expects one parameter");
        }

        if (CurTok == '(') {
            return LogErrorStmt("Gate '" + gateName + "' expects exactly one parameter");
        }

        std::vector<std::pair<std::string, int>> targets;

        // Parse and validate all targets
        while (true) {
            std::pair<std::string, int> target;
            if (!parseTarget(target, TargetKind::Quantum, "gate '" + gateName + "'")) {
                return nullptr;
            }
            targets.push_back(std::move(target));

            // Stop if there is no comma, otherwise consume it and continue parsing targets
            if (CurTok != ',') {
                break;
            }

            getNextToken();
        }

        // Verify the gate has the correct number of targets
        if (targets.size() != gateTargetCount(gateName)) {
            return LogErrorStmt("Gate '" + gateName + "' expects " + std::to_string(gateTargetCount(gateName)) + " target qubit(s)");
        }

        // Consume ';'
        if (CurTok != ';') return LogErrorStmt("Expected ';' after gate application");
        getNextToken();

        return std::make_unique<GateAST>(gateName, std::move(params), std::move(targets));
    }

    std::unique_ptr<StatementAST> Parser::parseMeasure() {
        // Consume 'measure'
        getNextToken();

        // Parse source
        std::pair<std::string, int> qubit;
        if (!parseTarget(qubit, TargetKind::Quantum, "measurement source")) return nullptr;

        // Consume '->'
        if (CurTok != '-') {
            return LogErrorStmt("Expected '->' after measure target");
        }
        getNextToken();

        if (CurTok != '>') {
            return LogErrorStmt("Expected '->' after measure target");
        }
        getNextToken();

        // Parse target
        std::pair<std::string, int> cbit;
        if (!parseTarget(cbit, TargetKind::Classical, "measurement destination")) return nullptr;

        // Consume ';'
        if (CurTok != ';') return LogErrorStmt("Expected ';' after measure statement");
        getNextToken();

        return std::make_unique<MeasureAST>(qubit, cbit);
    }

    std::unique_ptr<StatementAST> Parser::parseStatement() {
        switch (CurTok) {
            case tok_openqasm:      return parseOpenQASMVersion();
            case tok_include:       return parseInclude();
            case tok_qreg:
            case tok_creg:          return parseRegisterDecl();
            case tok_measure:       return parseMeasure();
            case tok_identifier:    return parseGate();
            default:
                return LogErrorStmt("Unknown statement in OpenQASM file");
        }
    }

    std::unique_ptr<ProgramAST> Parser::parseProgram() {
        auto program = std::make_unique<ProgramAST>();

        while (CurTok != tok_eof) {
            if (auto stmt = parseStatement()) {
                program->addStatement(std::move(stmt));
            } else {
                // Recovery: discard tokens until the next ';' or the end of the file is found
                while (CurTok != tok_eof && CurTok != ';') {
                    getNextToken();
                }

                // If we stopped on a ';', we consume it to prepare for the next instruction
                if (CurTok == ';') {
                    getNextToken();
                }
            }
        }

        return program;
    }
}