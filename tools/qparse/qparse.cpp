#include "Frontend/Lexer.h"
#include "Frontend/Parser.h"
#include "Frontend/MLIRGen.h"
#include "Dialect/QuantumDialect.h"

#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>

using namespace quantum;

void printToken(int tok, const Lexer& lexer) {
    std::cout << "Line " << std::left << std::setw(5) << lexer.getLine() 
              << " Col " << std::left << std::setw(4) << lexer.getCol() << " | ";
    
    switch(tok) {
        case tok_eof:        std::cout << "EOF\n"; break;
        case tok_qreg:       std::cout << "Keyword: qreg\n"; break;
        case tok_creg:       std::cout << "Keyword: creg\n"; break;
        case tok_measure:    std::cout << "Keyword: measure\n"; break;
        case tok_pi:         std::cout << "Keyword: pi\n"; break;
        case tok_openqasm:   std::cout << "Keyword: OPENQASM\n"; break;
        case tok_include:    std::cout << "Keyword: include\n"; break;
        case tok_identifier: std::cout << "Identifier: " << lexer.getIdentifierStr() << "\n"; break;
        case tok_integer:    std::cout << "Integer: " << lexer.getNumVal() << "\n"; break;
        case tok_float:      std::cout << "Float: " << lexer.getFloatVal() << "\n"; break;
        case tok_string:     std::cout << "String: \"" << lexer.getStringVal() << "\"\n"; break;
        default:             std::cout << "Symbol: '" << static_cast<char>(tok) << "'\n"; break;
    }
}

int main(int argc, char* argv[]) {
    // Args check
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_file.qasm> [--dump-ast] [--emit-mlir]\n";
        return 1;
    }

    // Parse command line arguments
    std::string filename = "";
    bool dumpAST = false;
    bool emitMLIR = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--dump-ast") {
            dumpAST = true;
        } else if (arg == "--emit-mlir") {
            emitMLIR = true;
        } else if (arg[0] != '-') {
            filename = arg;
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            return 1;
        }
    }

    // Checks for file path arg
    if (filename.empty()) {
        std::cerr << "Error: No input file specified.\n";
        return 1;
    }

    // File opening
    std::ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        std::cerr << "Error: unable to open file " << filename << "\n";
        return 1;
    }

    // Lexer initialization
    Lexer lexer(inputFile);

    // MLIR emission (--emit-mlir)
    if (emitMLIR) {
        Parser parser(lexer);
        auto ast = parser.parseProgram();
        if (!ast || parser.hasError()) {
            std::cerr << "Parsing failed.\n";
            return 1;
        }
        
        mlir::MLIRContext context;
        context.getOrLoadDialect<mlir::quantum::QuantumDialect>();
        context.getOrLoadDialect<mlir::func::FuncDialect>();
        
        auto module = quantum::mlirGen(context, *ast);
        if (!module) {
            std::cerr << "MLIR generation failed.\n";
            return 1;
        }
        
        module->print(llvm::outs());
        return 0;
    }

    // AST dumping (--dump-ast)
    if (dumpAST) {
        std::cout << "===== Parsing and AST Dump of " << filename << " =====\n";
        
        Parser parser(lexer);
        auto ast = parser.parseProgram();
        
        if (ast && !parser.hasError()) {
            ast->dump();
        } else {
            std::cerr << "\nParsing failed due to syntax errors.\n";
            return 1;
        }
    } else if (!emitMLIR) { // Tokens printing
        std::cout << "===== Lexical Analysis of " << filename << " =====\n";

        int tok;
        do {
            tok = lexer.getToken();
            if (tok != tok_error) {
                printToken(tok, lexer);
            }
        } while (tok != tok_eof && tok != tok_error);
    }

    return 0;
}