#include "Frontend/AST.h"

#include <iostream>

namespace quantum {

    static void printIndent(int indent) {
        for (int i = 0; i < indent; ++i) {
            std::cout << " ";
        }
    }

    void BinaryOpAST::dump(int indent) const {
        printIndent(indent);
        std::cout << "BinaryOp: '" << Op << "'\n";
        LHS -> dump(indent + 2);
        RHS -> dump(indent + 2);
    }

    void FloatExprAST::dump(int indent) const {
        printIndent(indent);
        std::cout << "FloatLiteral: " << Val << "\n";
    }

    void PiExpressionAST::dump(int indent) const {
        printIndent(indent);
        std::cout << "PiLiteral\n";
    }

    void RegisterDeclarationAST::dump(int indent) const {
        printIndent(indent);
        std::cout << (IsQuantum ? "QRegDecl" : "CRegDecl") 
                  << " '" << Name << "' [" << Size << "]\n";
    }

    void GateAST::dump(int indent) const {
        printIndent(indent);
        std::cout << "Gate '" << Name << "'\n";
        
        if (!Params.empty()) {
            printIndent(indent + 2);
            std::cout << "Params:\n";
            for (const auto& param : Params) {
                param -> dump(indent + 4);
            }
        }
        
        if (!Targets.empty()) {
            printIndent(indent + 2);
            std::cout << "Targets:\n";
            for (const auto& target : Targets) {
                printIndent(indent + 4);
                std::cout << target.first << "[" << target.second << "]\n";
            }
        }
    }

    void MeasureAST::dump(int indent) const {
        printIndent(indent);
        std::cout << "Measure: " 
                  << Qubit.first << "[" << Qubit.second << "] -> "
                  << ClassicalBit.first << "[" << ClassicalBit.second << "]\n";
    }

    void VersionDeclarationAST::dump(int indent) const {
        printIndent(indent);
        std::cout << "OpenQASMVersion: " << Version << "\n";
    }

    void IncludeDeclarationAST::dump(int indent) const {
        printIndent(indent);
        std::cout << "Include: \"" << FileName << "\"\n";
    }

    void ProgramAST::dump(int indent) const {
        std::cout << "ProgramAST\n";
        
        for (const auto& stmt : Statements) {
            stmt -> dump(indent + 2);
        }
    }

}