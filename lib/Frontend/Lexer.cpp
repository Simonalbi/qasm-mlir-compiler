#include "Frontend/Lexer.h"
#include <cctype>
#include <cstdlib>
#include <cstring>

namespace quantum {

    int Lexer::advance() {
        LastChar = Input.get();
        if (LastChar == '\n') {
            Line++;
            Column = 0;
        } else {
            Column++;
        }
        
        return LastChar;
    }

    void Lexer::printLexicalError(const std::string &msg) const {
        fprintf(stderr, "Lexical Error at Line: %d, Col: %d: %s\n", CurrentTokenLine, CurrentTokenCol, msg.c_str());
    }

    int Lexer::getIdentifier() {
        IdentifierStr = LastChar;

        while (isalnum((advance())) || LastChar == '_') {
            IdentifierStr += LastChar;
        }

        if (IdentifierStr == "qreg")    return tok_qreg;
        if (IdentifierStr == "creg")    return tok_creg;
        if (IdentifierStr == "measure") return tok_measure;
        if (IdentifierStr == "pi")      return tok_pi;
        if (IdentifierStr == "OPENQASM") return tok_openqasm;
        if (IdentifierStr == "include") return tok_include;

        return tok_identifier;
    }

    int Lexer::getNumber() {
        std::string NumStr;
        bool hasDot = (LastChar == '.');
        
        do {
            NumStr += LastChar;
            advance();

            if (LastChar == '.') {
                // Stop if a second decimal point is encountered the parser will handle the error
                if (hasDot) {
                    break;
                }
                hasDot = true;
            }
        } while (isdigit(LastChar) || LastChar == '.');

        if (hasDot) {
            FloatVal = strtod(NumStr.c_str(), nullptr);
            return tok_float;
        } else {
            NumVal = strtol(NumStr.c_str(), nullptr, 10);
            return tok_integer;
        }
    }

    int Lexer::getComment() {
        int nextChar = Input.peek();
        if (nextChar == '/') {
            do {
                advance();
            } while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');
        
            if (LastChar != EOF) {
                return getToken();
            }
            return tok_eof;
        }
        
        // If it's just a single slash, return it as its ASCII value
        int ThisChar = LastChar;
        advance();
        return ThisChar;
    }

    int Lexer::getString() {
        StringVal = "";

        // Skip the opening quote '"'
        advance();
        
        while (LastChar != '"' && LastChar != EOF && LastChar != '\n' && LastChar != '\r') {
            StringVal += LastChar;
            advance();
        }

        if (LastChar == '\n' || LastChar == '\r' || LastChar == EOF) {
            printLexicalError("Unterminated string: missing closing quote.");
            return tok_error;
        }
        
        // Skip the closing quote '"'
        if (LastChar == '"') {
            advance();
        }
        
        return tok_string;
    }

    int Lexer::getToken() {
        // Skip any whitespace characters
        while (isspace(LastChar)) {
            advance();
        }

        // Record the current line and column for error reporting
        CurrentTokenLine = Line;
        CurrentTokenCol = Column;

        if (isalpha(LastChar) || LastChar == '_') {
            return getIdentifier();
        }

        if (isdigit(LastChar) || LastChar == '.') {
            return getNumber();
        }

        if (LastChar == '"') {
            return getString();
        }

        if (LastChar == '/') {
            return getComment();
        }

        // Check for End-Of-File (EOF)
        if (LastChar == EOF || Input.eof()) {
            return tok_eof;
        }

        int ThisChar = LastChar;
        advance();

        // Cheks for OpenQASM 2.0 valid symbols based on the current subset (https://openqasm.com/grammar/index.html)
        if (strchr("()[];,->*/", ThisChar)) {
            return ThisChar;
        }

        // The character is not a valid symbol (Lexical error)
        std::string errMsg = "Unrecognized character: '";
        errMsg += (char)ThisChar;
        errMsg += "'";
        printLexicalError(errMsg);

        return tok_error;
    }

}