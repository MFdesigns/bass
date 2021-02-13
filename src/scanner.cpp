// ======================================================================== //
// Copyright 2020 Michel Fäh
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ======================================================================== //

#include "scanner.hpp"
#include "asm/asm.hpp"
#include "asm/encoding.hpp"
#include "cli.hpp"
#include <iomanip>
#include <iostream>

/**
 * Constructs a new Scanner instance
 * @param src Pointer to the source file
 * @param outTokens [out] Pointer to the output token vector
 */
Scanner::Scanner(SourceFile* src, std::vector<Token>* outTokens)
    : Src(src), Tokens(outTokens) {
    Cursor = 0;
    CursorLineRow = 1;
    CursorLineColumn = 1;
}

/**
 * Increments the line row
 */
void Scanner::incLineRow() {
    CursorLineRow++;
    CursorLineColumn = 0;
};

/**
 * Increments the cursor
 */
void Scanner::incCursor() {
    Cursor++;
    CursorLineColumn++;
}

/**
 * Returns the char at the current Cursor index from the source file after
 * increasing the Cursor
 * @return On success returns char. At end of file returns 0
 */
char Scanner::eatChar() {
    incCursor();
    if (Cursor >= Src->getSize()) {
        return 0;
    }
    char c = 0;
    Src->getChar(Cursor, c);
    return c;
}

/**
 * Skips ahead in the source file
 * Note: This does not take new lines into account and will not increase the
 * line row if a \n is skipped
 * @param count Amount of chars to skip
 */
void Scanner::skipChar(uint32_t count) {
    Cursor += count;
    CursorLineColumn += count;
}

/**
 * Peeks the next token without increasing the Cursor
 * @return On success returns char. At end of file returns 0
 */
char Scanner::peekChar() {
    if (Cursor + 1 >= Src->getSize()) {
        return 0;
    }
    char c = 0;
    Src->getChar(Cursor + 1, c);
    return c;
}

/**
 * Checks if a token is an instruction. If so it will attach the instruction
 * encoding id to the tag
 * @param token The token to check
 * @param tag [out] If token is an instruction tag will hold the instruction
 * encoding id
 * @return On instruction will return true otherwise false
 */
bool Scanner::isInstruction(std::string& token, uint8_t& tag) {
    bool isInstr = false;
    auto iter = Asm::INSTR_NAMES.find(token);
    if (iter != Asm::INSTR_NAMES.end()) {
        tag = iter->second;
        isInstr = true;
    }

    return isInstr;
}

/**
 * Checks if a token is a type info. If so it will attach the type id to the tag
 * @param token The token to check
 * @param tag [out] If token is a type info tag will hold the type id
 * @return On type will return true otherwise false
 */
bool Scanner::isTypeInfo(std::string& token, uint8_t& tag) {
    bool isTypeInfo = false;
    auto iter = UVM_TYPE_DEFS.find(token);
    if (iter != UVM_TYPE_DEFS.end()) {
        tag = iter->second;
        isTypeInfo = true;
    }
    return isTypeInfo;
}

/**
 * Checks if a token is a register. If so it will attach the register id to the
 * tag
 * @param token The token to check
 * @param tag [out] If token is a register tag will hold the register id
 * @return On register will return true otherwise false
 */
bool Scanner::isRegister(std::string& token, uint8_t& tag) {
    bool isRegister = false;
    auto iter = ASM_REGISTERS.find(token);
    if (iter != ASM_REGISTERS.end()) {
        tag = iter->second;
        isRegister = true;
    }
    return isRegister;
}

/**
 * Skips the current line where the Cursor is on
 */
void Scanner::skipLine() {
    char peek = peekChar();
    while (peek != 0 && peek != '\n') {
        incCursor();
        peek = peekChar();
    }
}

/**
 * Scans a word which starts with [a-zA-Z] contains [a-zA-Z_0-9]+ terminated by
 * [ \t{\n\r,\]+\-*:=]
 * @param outSize The word size
 * @return On valid word returns true otherwise false
 */
bool Scanner::scanWord(uint32_t& outSize) {
    char c = ' ';
    Src->getChar(Cursor, c);
    bool validWord = true;
    char peek = peekChar();

    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
        outSize++;
    } else {
        return false;
    }

    while (peek != '\t' && peek != ' ' && peek != '{' && peek != '\n' &&
           peek != '\r' && peek != ',' && peek != ']' && peek != '+' &&
           peek != '-' && peek != '*' && peek != ':' && peek != '=') {
        c = eatChar();
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' ||
            (c >= '0' && c <= '9')) {
            outSize++;
            peek = peekChar();
        } else {
            validWord = false;
            break;
        }
    }

    return validWord;
}

/**
 * Scans a string declared by "..."
 * @param outSize [out] Size of the string
 * @return On valid string returns true otherwise false
 */
bool Scanner::scanString(uint32_t& outSize) {
    bool validString = true;

    // Get first char and peek
    char c = ' ';
    Src->getChar(Cursor, c);
    char peek = peekChar();
    outSize++;

    // String cannot not be on multiple lines
    bool strClosed = false;
    while (!strClosed) {
        c = eatChar();
        if (c == '\n') {
            validString = false;
            break;
        }

        if (c == '"') {
            strClosed = true;
        }
        outSize++;
        peek = peekChar();
    }

    return validString;
}

/**
 * Scans a number
 * @param outSize [out] Size of the number string
 * @return On valid number retuns true otherwise false
 */
bool Scanner::scanNumber(uint32_t& outSize, bool& isFloat) {
    bool validNumber = true;

    // Get first char and peek
    char c = ' ';
    Src->getChar(Cursor, c);
    char peek = peekChar();

    // Check if number has hex prefix 0x
    if (c == '0' && peek == 'x') {
        outSize += 2;
        incCursor();

        // Parse hex number until terminated by [ \n\t,\r\]]
        while (peek != 0 && peek != ' ' && peek != '\n' && peek != '\t' &&
               peek != ',' && peek != '\r' && peek != ']') {
            c = eatChar();
            // Check for valid hex number [0-9a-fA-F]+
            if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
                (c >= 'A' && c <= 'F')) {
                outSize++;
                peek = peekChar();
            } else {
                validNumber = false;
                break;
            }
        }

    } else {
        bool hasExponent = false;
        // Parse dec or float number until terminated by [ \n\t,\r\]]
        while (validNumber) {
            // Valid dec or float number [0-9]+
            if (c >= '0' && c <= '9') {
                outSize++;
            } else if (c == '.') {
                // If number contains comma '.' number is a float. If number
                // contains more than one comma '.' number is invalid
                if (isFloat) {
                    validNumber = false;
                    break;
                }

                isFloat = true;
                outSize++;
            } else if (c == 'e' || c == 'E') {
                // If preceding number have a comma '.' in it number is a float otherwise an integer
                if (!isFloat) {
                    validNumber = false;
                    break;
                }

                // e might be followed by + or - sign
                if (peek == '+' || peek == '-') {
                    c = eatChar();
                    outSize++;
                }
                hasExponent = true;
                outSize++;
            } else {
                validNumber = false;
                break;
            }

            if (peek == 0 || peek == ' ' || peek == '\n' || peek == '\t' ||
                peek == ',' || peek == '\r' || peek == ']') {
                break;
            }
            c = eatChar();
            peek = peekChar();
        }
    }

    return validNumber;
}

/**
 * Checks what type of token a word is
 * @param word Token string
 * @param tag  [out] Pointer to tag information can be nullptr
 * @return Type of token
 */
TokenType Scanner::identifyWord(std::string& word, uint8_t* tag) {
    TokenType type = TokenType::IDENTIFIER;
    uint8_t tmpTag = 0;
    if (isInstruction(word, tmpTag)) {
        type = TokenType::INSTRUCTION;
    } else if (isTypeInfo(word, tmpTag)) {
        type = TokenType::TYPE_INFO;
    } else if (isRegister(word, tmpTag)) {
        type = TokenType::REGISTER_DEFINITION;
    }

    if (tag != nullptr) {
        *tag = tmpTag;
    }

    // If the token is not an instruction, type info or register def then it
    // must be an identifier which is the default value of type
    return type;
}

/**
 * Scans the source file and output tokens
 * @return On success returns true otherwise false
 */
bool Scanner::scanSource() {
    bool validSource = true;
    char currChar = 0;
    Src->getChar(Cursor, currChar);

    while (currChar != 0) {
        // Take a snapshot of the current token position before parsing
        // further and increasing the cursor
        uint32_t tokPos = Cursor;
        uint32_t tokLineRow = CursorLineRow;
        uint32_t tokLineColumn = CursorLineColumn;

        // Check if char is a valid word start [a-zA-Z_]
        if ((currChar >= 'A' && currChar <= 'Z') ||
            (currChar >= 'a' && currChar <= 'z') || currChar == '_') {
            // Get complete word
            uint32_t wordSize = 0;
            bool validWord = scanWord(wordSize);
            if (!validWord) {
                validSource = false;
                printError(Src, tokPos, wordSize, tokLineRow, tokLineColumn,
                           "Unexpected character in identifer");
                skipLine();
                currChar = eatChar();
                continue;
            }

            // Get word as string
            std::string token{};
            Src->getSubStr(tokPos, wordSize, token);

            // Get word type and add it to tokens array
            uint8_t tag = 0;
            TokenType type = identifyWord(token, &tag);
            Tokens->emplace_back(type, tokPos, wordSize, tokLineRow,
                                 tokLineColumn, tag);
            currChar = eatChar();

        } else if (currChar >= '0' && currChar <= '9') {
            uint32_t numSize = 0;
            bool isFloat = false;
            bool validNum = scanNumber(numSize, isFloat);
            if (!validNum) {
                validSource = false;
                printError(Src, tokPos, numSize, tokLineRow, tokLineColumn,
                           "Unexpected character in number");
                skipLine();
                currChar = eatChar();
                continue;
            }

            TokenType type = TokenType::INTEGER_NUMBER;
            if (isFloat) {
                type = TokenType::FLOAT_NUMBER;
            }
            Tokens->emplace_back(type, tokPos, numSize, tokLineRow,
                                 tokLineColumn, 0);
            currChar = eatChar();

        } else if (currChar == ' ' || currChar == '\t') {
            // Skip whitespace
            currChar = eatChar();
        } else {
            switch (currChar) {
            case '+':
                Tokens->emplace_back(TokenType::PLUS_SIGN, Cursor, 1,
                                     CursorLineRow, CursorLineColumn, 0);
                break;
            case '-':
                Tokens->emplace_back(TokenType::MINUS_SIGN, Cursor, 1,
                                     CursorLineRow, CursorLineColumn, 0);
                break;
            case '*':
                Tokens->emplace_back(TokenType::ASTERISK, Cursor, 1,
                                     CursorLineRow, CursorLineColumn, 0);
                break;
            case ',':
                Tokens->emplace_back(TokenType::COMMA, Cursor, 1, CursorLineRow,
                                     CursorLineColumn, 0);
                break;
            case '[':
                Tokens->emplace_back(TokenType::LEFT_SQUARE_BRACKET, Cursor, 1,
                                     CursorLineRow, CursorLineColumn, 0);
                break;
            case ']':
                Tokens->emplace_back(TokenType::RIGHT_SQUARE_BRACKET, Cursor, 1,
                                     CursorLineRow, CursorLineColumn, 0);
                break;
            case '{':
                Tokens->emplace_back(TokenType::LEFT_CURLY_BRACKET, Cursor, 1,
                                     CursorLineRow, CursorLineColumn, 0);
                break;
            case '}':
                Tokens->emplace_back(TokenType::RIGHT_CURLY_BRACKET, Cursor, 1,
                                     CursorLineRow, CursorLineColumn, 0);
                break;
            case ':':
                Tokens->emplace_back(TokenType::COLON, Cursor, 1, CursorLineRow,
                                     CursorLineColumn, 0);
                break;
            case '=':
                Tokens->emplace_back(TokenType::EQUALS_SIGN, Cursor, 1,
                                     CursorLineRow, CursorLineColumn, 0);
                break;
            case '"': {
                uint32_t strSize = 0;
                bool validString = scanString(strSize);
                if (!validString) {
                    printError(Src, tokPos, strSize, tokLineRow, tokLineColumn,
                               "Unexpected character in string");
                    skipLine();
                    currChar = eatChar();
                    continue;
                }
                Tokens->emplace_back(TokenType::STRING, tokPos, strSize,
                                     tokLineRow, tokLineColumn, 0);
                break;
            }
            case '@': {
                // Start at 1 to include @ sign
                uint32_t labelSize = 1;
                incCursor();
                bool validWord = scanWord(labelSize);
                if (!validWord) {
                    validSource = false;
                    printError(Src, tokPos, labelSize, tokLineRow,
                               tokLineColumn,
                               "Unexpected character in label identifer");
                    skipLine();
                    currChar = eatChar();
                    continue;
                }

                // Get label as string
                std::string label{};
                Src->getSubStr(tokPos, labelSize, label);

                // Get word type and add it to tokens array
                TokenType type = identifyWord(label, nullptr);
                if (type != TokenType::IDENTIFIER) {
                    printError(Src, tokPos, labelSize, tokLineRow,
                               tokLineColumn,
                               "Keyword inside label identifier");
                    skipLine();
                    currChar = eatChar();
                    continue;
                }

                Tokens->emplace_back(TokenType::LABEL_DEF, tokPos, labelSize,
                                     tokLineRow, tokLineColumn, 0);
            } break;
            case '\r':
                // Skip CR
                break;
            case '\n': {
                // Only add EOL tokens once in a row
                Token* last = nullptr;
                if (!Tokens->empty()) {
                    last = &Tokens->back();
                }
                if (last != nullptr && last->Type != TokenType::EOL) {
                    Tokens->emplace_back(TokenType::EOL, Cursor, 1,
                                         CursorLineRow, CursorLineColumn, 0);
                }
                incLineRow();
            } break;
            case '/': {
                char peek = peekChar();
                if (peek == '/') {
                    incCursor();
                    peek = peekChar();
                    while (peek != 0 && peek != '\n') {
                        incCursor();
                        peek = peekChar();
                    }
                } else {
                    validSource = false;
                    skipLine();
                }
            } break;
            default:
                printError(Src, tokPos, 1, tokLineRow, tokLineColumn,
                           "Unexpected character");
                skipLine();
                validSource = false;
                break;
            }
            currChar = eatChar();
        }
    }

    // Add end of file token
    Tokens->emplace_back(TokenType::END_OF_FILE, Cursor - 1, 1, CursorLineRow,
                         CursorLineColumn, 0);

    return validSource;
}
