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
#include <iomanip>
#include <iostream>

Source::Source(uint8_t* data, uint32_t size) : Data(data), Size(size) {}

Source::~Source() {
    delete[] Data;
}

uint32_t Source::getSize() {
    return Size;
}

uint8_t* Source::getData() {
    return Data;
}

bool Source::getChar(uint32_t index, uint8_t& c) {
    if (index < Size) {
        c = Data[index];
        return true;
    } else {
        return false;
    }
}

bool Source::getSubStr(uint32_t index, uint32_t size, std::string& out) {
    if (index < Size && index + size < Size) {
        out.clear();
        out.reserve(size);
        uint8_t c;
        for (auto i = 0; i < size; i++) {
            getChar(index + i, c);
            out.push_back(c);
        }
        return true;
    } else {
        return false;
    }
}

bool Source::getLine(uint32_t index, std::string& out, uint32_t& lineIndex) {
    if (index >= Size) {
        return false;
    }

    // Find the begining of the line
    bool foundSOL = false;
    while (!foundSOL && (int32_t)index - 1 > 0) {
        if (Data[index - 1] == '\n') {
            foundSOL = true;
        } else {
            index--;
        }
    }
    lineIndex = index;

    // Parse the whole line
    uint8_t c = Data[index];
    while (c != '\n' && index < Size) {
        index++;
        out.push_back(c);
        c = Data[index];
    }

    return true;
}

Scanner::Scanner(Source* src, std::vector<Token>* outTokens)
    : Src(src), Tokens(outTokens) {
    Cursor = 0;
    CursorLineRow = 1;
    CursorLineColumn = 1;
}

void Scanner::incLineRow() {
    CursorLineRow++;
    CursorLineColumn = 0;
};

void Scanner::incCursor() {
    Cursor++;
    CursorLineColumn++;
}

bool Scanner::eatChar(uint8_t& out) {
    bool eof = false;
    if (Cursor + 1 >= Src->getSize()) {
        eof = true;
    } else {
        uint8_t* data = Src->getData();
        Cursor++;
        CursorLineColumn++;
        out = data[Cursor];
    }
    return eof;
}

bool Scanner::eatChars(uint32_t count, uint8_t& out) {
    bool eof = false;
    if (Cursor + count >= Src->getSize()) {
        eof = true;
    } else {
        uint8_t* data = Src->getData();
        Cursor += count;
        CursorLineColumn += count;
        out = data[Cursor];
    }
    return eof;
}

bool Scanner::peekChar(uint8_t& out) {
    bool eof = false;
    if (Cursor + 1 >= Src->getSize()) {
        eof = true;
    } else {
        uint8_t* data = Src->getData();
        out = data[Cursor + 1];
    }
    return eof;
}

void Scanner::addToken(TokenType type,
                       uint32_t index,
                       uint32_t lineRow,
                       uint32_t lineColumn,
                       uint32_t size,
                       uint8_t tag) {
    Tokens->emplace_back(type, index, size, lineRow, lineColumn, tag);
}

bool Scanner::isInstruction(std::string& token, uint8_t& tag) {
    bool isInstr = false;
    auto iter = Asm::INSTR_NAMES.find(token);
    if (iter != Asm::INSTR_NAMES.end()) {
        tag = iter->second;
        isInstr = true;
    }

    return isInstr;
}

bool Scanner::isTypeInfo(std::string& token, uint8_t& tag) {
    for (const auto& type : UVM_TYPE_DEFS) {
        if (token == type.Str) {
            tag = type.Id;
            return true;
        }
    }
    return false;
}

/**
 * Checks is a token is a register. If so it will attach the register id to the tag
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

void Scanner::throwError(const char* msg, uint32_t start) {
    std::string line;
    uint32_t lineIndex = 0;
    Src->getLine(start, line, lineIndex);
    std::string lineNr = std::to_string(lineIndex);

    uint32_t errOffset = start - lineIndex;
    uint8_t c = 0;
    Src->getChar(Cursor, c);

    std::string lineNumber = std::to_string(CursorLineRow);
    std::cout << "[Syntax Error] " << msg << " at Ln " << CursorLineRow
              << ", Col " << CursorLineColumn << " at char '" << c << "' (U+"
              << (uint16_t)c << ")\n"
              << "  " << lineNumber << " | " << line << '\n'
              << "  " << std::setw(2 + lineNumber.size()) << std::setfill(' ')
              << " |" << std::setw(errOffset + 1) << std::setfill(' ') << ' '
              << std::setw(CursorLineColumn - errOffset) << std::setfill('~')
              << '~' << '\n'
              << std::endl;
}

bool Scanner::scanWord(uint32_t& outSize) {
    uint8_t c = ' ';
    Src->getChar(Cursor, c);
    bool terminated = false;
    do {
        if (c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z' || c == '_' ||
            c >= '0' && c <= '9') {
            outSize++;
            // check if next char terminates identifier if not eat next
            // char
            uint8_t peek;
            terminated = !Src->getChar(Cursor + 1, peek);
            if (!terminated &&
                (peek == '\t' || peek == ' ' || peek == '{' || peek == '\n' ||
                 peek == '\r' || peek == ',' || peek == ']' || peek == '+' ||
                 peek == '-' || peek == '*')) {
                terminated = true;
            } else if (!terminated) {
                eatChar(c);
            }
        } else {
            return false;
        }
    } while (!terminated);
    return true;
}

void Scanner::skipLine() {
    uint8_t peek = ' ';
    bool terminated = peekChar(peek);
    while (!terminated && peek != '\n') {
        incCursor();
        terminated = peekChar(peek);
    }
}

bool Scanner::scanSource() {
    uint8_t* data = Src->getData();
    uint8_t c = data[Cursor]; // char at current cursor position
    bool eof = false;
    bool valid = true;
    while (!eof) {
        // Take a snapshot of the current token position before parsing
        // further and increasing the cursor
        uint32_t tokPos = Cursor;
        uint32_t tokLineRow = CursorLineRow;
        uint32_t tokLineColumn = CursorLineColumn;
        // Handle valid identifier start and figure out if identifier could be
        // instruction, type info or register
        if (c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z' || c == '_') {
            uint32_t tokSize = 0;
            bool validId = scanWord(tokSize);
            if (!validId) {
                throwError("Unexpected token in identifier", tokPos);
                skipLine();
                valid = false;
                eof = eatChar(c);
                continue;
            }

            std::string token{};
            Src->getSubStr(tokPos, tokSize, token);

            // Check if token is instruction
            uint8_t tag = 0;
            if (isInstruction(token, tag)) {
                addToken(TokenType::INSTRUCTION, tokPos, tokLineRow,
                         tokLineColumn, token.size(), tag);
            } else if (isTypeInfo(token, tag)) {
                addToken(TokenType::TYPE_INFO, tokPos, tokLineRow,
                         tokLineColumn, token.size(), tag);
            } else if (isRegister(token, tag)) {
                addToken(TokenType::REGISTER_DEFINITION, tokPos, tokLineRow,
                         tokLineColumn, token.size(), tag);
            } else {
                // If the token is not an instruciton, type info or register
                // then it must be an identifier
                addToken(TokenType::IDENTIFIER, tokPos, tokLineRow,
                         tokLineColumn, token.size(), 0);
            }
            eof = eatChar(c);

            // If char is whitespace just eat it and continue
        } else if (c == ' ' || c == '\t') {
            eof = eatChar(c);
        } else if (c >= '0' && c <= '9') {
            // TODO: Fix this ugly mess!
            std::string token;
            TokenType type = TokenType::INTEGER_NUMBER;
            bool terminated = false;
            bool validNumber = true;
            uint8_t peek;
            terminated = peekChar(peek);
            // Check for possible hex prefix
            if (c == '0' && peek == 'x') {
                // consume prefix
                token.append("0x");
                terminated = eatChars(2, c);
                do {
                    terminated = peekChar(peek);
                    if (c >= '0' && c <= '9' || c >= 'a' && c <= 'f' ||
                        c >= 'A' && c <= 'F') {
                        token.push_back(c);
                        if (peek == '\t' || peek == ' ' || peek == ',' ||
                            peek == '\n' || peek == '\r' || peek == ']') {
                            terminated = true;
                        } else {
                            terminated = eatChar(c);
                        }
                    } else {
                        throwError("Expected hex number",
                                   Cursor - token.size());
                        skipLine();
                        validNumber = false;
                        valid = false;
                    }

                } while (!terminated && validNumber);
            } else {
                do {
                    terminated = peekChar(peek);
                    if (c >= '0' && c <= '9') {
                        token.push_back(c);
                        if (peek == '\t' || peek == ' ' || peek == ',' ||
                            peek == '\n' || peek == '\r' || peek == ']') {
                            terminated = true;
                        } else {
                            terminated = eatChar(c);
                        }
                    } else if (c == '.' && type == TokenType::INTEGER_NUMBER) {
                        type = TokenType::FLOAT_NUMBER;
                        token.push_back(c);
                        if (peek == '\t' || peek == ' ' || peek == ',' ||
                            peek == '\n' || peek == '\r' || peek == ']') {
                            terminated = true;
                        } else {
                            terminated = eatChar(c);
                        }
                    } else if (c == '.' && type == TokenType::FLOAT_NUMBER) {
                        throwError("More than one decimal point",
                                   Cursor - token.size());
                        skipLine();
                        valid = false;
                        validNumber = false;
                    } else {
                        throwError("Expected number", Cursor - token.size());
                        skipLine();
                        valid = false;
                        validNumber = false;
                    }
                } while (!terminated && validNumber);
            }

            if (validNumber) {
                addToken(type, tokPos, tokLineRow, tokLineColumn, token.size(),
                         0);
            }
            eof = eatChar(c);

        } else {
            switch (c) {
            case '/': {
                uint8_t peek;
                bool terminated = peekChar(peek);
                if (peek == '/') {
                    eatChars(2, c);
                    while (!terminated && peek != '\n') {
                        eatChar(c);
                        terminated = peekChar(peek);
                    }
                } else {
                    throwError("Expected double back slash", Cursor);
                    skipLine();
                    valid = false;
                }
            } break;
            case '+':
                addToken(TokenType::PLUS_SIGN, Cursor, CursorLineRow,
                         CursorLineColumn, 1, 0);
                break;
            case '-':
                addToken(TokenType::MINUS_SIGN, Cursor, CursorLineRow,
                         CursorLineColumn, 1, 0);
                break;
            case '*':
                addToken(TokenType::ASTERISK, Cursor, CursorLineRow,
                         CursorLineColumn, 1, 0);
                break;
            case ',':
                addToken(TokenType::COMMA, Cursor, CursorLineRow,
                         CursorLineColumn, 1, 0);
                break;
            case '[':
                addToken(TokenType::LEFT_SQUARE_BRACKET, Cursor, CursorLineRow,
                         CursorLineColumn, 1, 0);
                break;
            case ']':
                addToken(TokenType::RIGHT_SQUARE_BRACKET, Cursor, CursorLineRow,
                         CursorLineColumn, 1, 0);
                break;
            case '{':
                addToken(TokenType::LEFT_CURLY_BRACKET, Cursor, CursorLineRow,
                         CursorLineColumn, 1, 0);
                break;
            case '}':
                addToken(TokenType::RIGHT_CURLY_BRACKET, Cursor, CursorLineRow,
                         CursorLineColumn, 1, 0);
                break;
            case '@': {
                uint32_t tokSize = 1;
                incCursor();
                bool validId = scanWord(tokSize);
                if (!validId) {
                    throwError("Unexpected token in label definition", tokPos);
                    skipLine();
                    eof = eatChar(c);
                    valid = false;
                    continue;
                }

                // Get token without @ sign
                std::string token{};
                Src->getSubStr(tokPos + 1, tokSize - 1, token);

                // Check if token is instruction
                uint8_t tag = 0; // Not used here
                if (isInstruction(token, tag)) {
                    throwError("Instruction keyword inside label definition",
                               tokPos);
                    skipLine();
                    valid = false;
                } else if (isTypeInfo(token, tag)) {
                    throwError("Type keyword inside label definition", tokPos);
                    skipLine();
                    valid = false;
                } else if (isRegister(token, tag)) {
                    throwError("Register keyword inside label definition",
                               tokPos);
                    skipLine();
                    valid = false;
                } else {
                    // If the token is not an instruciton, type info or register
                    // then it must be an identifier
                    addToken(TokenType::LABEL_DEF, tokPos, tokLineRow,
                             tokLineColumn, token.size() + 1, 0);
                }
            } break;
            case '\r':
                // Skip CR
                break;
            case '\n': {
                // Only add EOL tokens once in a row
                Token* last = nullptr;
                if (Tokens->size() > 0) {
                    last = &Tokens->back();
                }
                if (last != nullptr && last->Type != TokenType::EOL) {
                    addToken(TokenType::EOL, Cursor, CursorLineRow,
                             CursorLineColumn, 1, 0);
                }
                incLineRow();
            } break;
            default:
                throwError("Unexpected character", Cursor);
                skipLine();
                valid = false;
            }
            eof = eatChar(c);
        }
    }

    // TODO: Fix EOF position
    addToken(TokenType::END_OF_FILE, Cursor, CursorLineRow, CursorLineColumn, 1,
             0);

    return valid;
}
