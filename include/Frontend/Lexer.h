#ifndef FRONTEND_LEXER_H
#define FRONTEND_LEXER_H

#include <string>
#include <istream>

namespace quantum {

    enum Token {
        tok_eof = -1,
        tok_error = -12,

        // OpenQASM 2.0 tokens subset
        tok_qreg = -2,
        tok_creg = -3,
        tok_measure = -4,
        tok_pi = -5,
        tok_openqasm = -10,
        tok_include = -11,
        
        tok_identifier = -6,
        tok_integer = -7,
        tok_float = -8,
        tok_string = -9,
        
    };

    class Lexer {
        std::istream &Input;

        // Current analyzing character
        int LastChar = ' ';

        // Current Lexet state
        std::string IdentifierStr; // tok_identifier
        int NumVal;                // tok_integer
        double FloatVal;           // tok_float
        std::string StringVal;     // tok_string

        int Line = 1;
        int Column = 0;
        int CurrentTokenLine = 1;
        int CurrentTokenCol = 1;

        /**
         * @brief Reads the next character from the input stream.
         * 
         * This method consumes a single character from the input stream and updates
         * the internal 'LastChar' state. It also maintains the current line and column
         * position to facilitate accurate error reporting.
         * 
         * @return int The ASCII value of the character read, or EOF.
         */
        int advance();

        /**
         * @brief Logs a lexical error to standard error.
         * 
         * Prints a formatted error message including the current line
         * and column number where the lexical error occurred.
         * 
         * @param msg The error message to be printed.
         */
        void printLexicalError(const std::string &msg) const;

        /**
         * @brief Gets and parses identifiers and OpenQASM keywords.
         * 
         * @return int The token identifier for keywords (e.g., tok_qreg) or tok_identifier.
         */
        int getIdentifier();

        /**
         * @brief Gets and parses numeric literals (integers and floating-point).
         * 
         * @return int The token identifier tok_integer or tok_float.
         */
        int getNumber();

        /**
         * @brief Gets the next token after processing single-line comments.
         * 
         * @return int The next valid token after skipping the comment, or the ASCII value of '/' if not a comment.
         */
        int getComment();

        /**
         * @brief Gets and parses string literals enclosed in double quotes.
         * 
         * @return int The token identifier tok_string.
         */
        int getString();

        public:
            Lexer(std::istream &In) : Input(In) {}

            /**
             * @brief Reads and returns the next token from the input.
             * 
             * Scans the input stream, skipping whitespace and comments, to identify
             * and return the next valid OpenQASM token.
             * 
             * @return int The token identifier (negative for keywords/types, positive ASCII for characters).
             */
            int getToken();

            // Getter
            const std::string &getIdentifierStr() const { return IdentifierStr; }
            int getNumVal() const { return NumVal; }
            double getFloatVal() const { return FloatVal; }
            const std::string &getStringVal() const { return StringVal; }
            int getLine() const { return CurrentTokenLine; }
            int getCol() const { return CurrentTokenCol; }
    };
}

#endif