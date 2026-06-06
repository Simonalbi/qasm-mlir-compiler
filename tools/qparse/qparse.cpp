#include <iostream>
#include <fstream>
#include "Frontend/Lexer.h"

using namespace quantum;

void printToken(int tok, const Lexer& lexer) {
    std::cout << "Line " << lexer.getLine() << ", Col " << lexer.getCol() << " \t| ";
    
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
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_file.qasm>\n";
        return 1;
    }

    std::ifstream inputFile(argv[1]);
    if (!inputFile.is_open()) {
        std::cerr << "Error: unable to open file " << argv[1] << "\n";
        return 1;
    }

    Lexer lexer(inputFile);

    std::cout << "--- Starting Lexical Analysis of " << argv[1] << " ---\n";

    int tok;
    do {
        tok = lexer.getToken();
        if (tok != tok_error) {
            printToken(tok, lexer);
        }
    } while (tok != tok_eof && tok != tok_error);

    return 0;
}