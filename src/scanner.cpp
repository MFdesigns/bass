/**
 * Copyright 2020 Michel Fäh
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "scanner.hpp"
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
                       uint32_t size) {
    Tokens->emplace_back(type, index, size, lineRow, lineColumn);
}

bool Scanner::isInstruction(std::string& token) {
    bool isInstr = false;
    for (const auto& instr : INSTRUCTIONS) {
        if (token == instr) {
            isInstr = true;
            break;
        }
    }
    return isInstr;
}

bool Scanner::isTypeInfo(std::string& token) {
    bool isTypeInfo = false;
    for (const auto& typeInfo : TYPE_INFOS) {
        if (token == typeInfo) {
            isTypeInfo = true;
            break;
        }
    }
    return isTypeInfo;
}

bool Scanner::isRegister(std::string& token) {
    bool isRegister = false;
    for (const auto& reg : REGISTERS) {
        if (token == reg) {
            isRegister = true;
            break;
        }
    }
    return isRegister;
}

void Scanner::throwError(const char* msg, uint32_t start) {
    uint8_t* data = Src->getData();

    // Parse the whole line where the error occured
    std::string line{};
    uint32_t lineCursor = Cursor - (CursorLineColumn - 1);
    uint32_t errOffset = start - lineCursor;
    uint8_t c = data[lineCursor];
    while (c != '\n') {
        line.push_back(data[lineCursor]);
        lineCursor++;
        c = data[lineCursor];
    }

    std::string lineNumber = std::to_string(CursorLineRow);
    std::cout << "[Syntax Error] " << msg << " at Ln " << CursorLineRow
              << ", Col " << CursorLineColumn << " at char '" << data[Cursor]
              << "' (" << (uint16_t)data[Cursor] << ")\n"
              << "  " << lineNumber << " | " << line << '\n'
              << "  " << std::setw(2 + lineNumber.size()) << std::setfill(' ')
              << " |" << std::setw(errOffset + 1) << std::setfill(' ') << ' '
              << std::setw(CursorLineColumn - errOffset) << std::setfill('~')
              << '~' << '\n';
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
                 peek == ',' || peek == ']' || peek == '+' || peek == '-')) {
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

bool Scanner::scanSource() {
    uint8_t* data = Src->getData();
    uint8_t c = data[Cursor]; // char at current cursor position
    bool eof = false;
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
                return false;
            }

            std::string token{};
            Src->getSubStr(tokPos, tokSize, token);

            // Check if token is instruction
            if (isInstruction(token)) {
                addToken(TokenType::INSTRUCTION, tokPos, tokLineRow,
                         tokLineColumn, token.size());
            } else if (isTypeInfo(token)) {
                addToken(TokenType::TYPE_INFO, tokPos, tokLineRow,
                         tokLineColumn, token.size());
            } else if (isRegister(token)) {
                addToken(TokenType::REGISTER_DEFINITION, tokPos, tokLineRow,
                         tokLineColumn, token.size());
            } else {
                // If the token is not an instruciton, type info or register
                // then it must be an identifier
                addToken(TokenType::IDENTIFIER, tokPos, tokLineRow,
                         tokLineColumn, token.size());
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
                            peek == '\n' || peek == ']') {
                            terminated = true;
                        } else {
                            terminated = eatChar(c);
                        }
                    } else {
                        throwError("Expected hex number",
                                   Cursor - token.size());
                        return false;
                    }

                } while (!terminated);
            } else {
                do {
                    terminated = peekChar(peek);
                    if (c >= '0' && c <= '9') {
                        token.push_back(c);
                        if (peek == '\t' || peek == ' ' || peek == ',' ||
                            peek == '\n' || peek == ']') {
                            terminated = true;
                        } else {
                            terminated = eatChar(c);
                        }
                    } else if (c == '.' && type == TokenType::INTEGER_NUMBER) {
                        type = TokenType::FLOAT_NUMBER;
                        token.push_back(c);
                        if (peek == '\t' || peek == ' ' || peek == ',' ||
                            peek == '\n' || peek == ']') {
                            terminated = true;
                        } else {
                            terminated = eatChar(c);
                        }
                    } else if (c == '.' && type == TokenType::FLOAT_NUMBER) {
                        throwError("More than one decimal point",
                                   Cursor - token.size());
                        return false;
                    } else {
                        throwError("Expected number", Cursor - token.size());
                        return false;
                    }
                } while (!terminated);
            }

            addToken(type, tokPos, tokLineRow, tokLineColumn, token.size());
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
                    throwError("Unexpected token", Cursor);
                    return false;
                }
            } break;
            case '+':
                addToken(TokenType::PLUS_SIGN, Cursor, CursorLineRow,
                         CursorLineColumn, 1);
                break;
            case '-':
                addToken(TokenType::MINUS_SIGN, Cursor, CursorLineRow,
                         CursorLineColumn, 1);
                break;
            case '*':
                addToken(TokenType::ASTERISK, Cursor, CursorLineRow,
                         CursorLineColumn, 1);
                break;
            case ',':
                addToken(TokenType::COMMA, Cursor, CursorLineRow,
                         CursorLineColumn, 1);
                break;
            case '[':
                addToken(TokenType::LEFT_SQUARE_BRACKET, Cursor, CursorLineRow,
                         CursorLineColumn, 1);
                break;
            case ']':
                addToken(TokenType::RIGHT_SQUARE_BRACKET, Cursor, CursorLineRow,
                         CursorLineColumn, 1);
                break;
            case '{':
                addToken(TokenType::LEFT_CURLY_BRACKET, Cursor, CursorLineRow,
                         CursorLineColumn, 1);
                break;
            case '}':
                addToken(TokenType::RIGHT_CURLY_BRACKET, Cursor, CursorLineRow,
                         CursorLineColumn, 1);
                break;
            case '@': {
                uint32_t tokSize = 1;
                incCursor();
                bool validId = scanWord(tokSize);
                if (!validId) {
                    throwError("Unexpected token in label definition", tokPos);
                    return false;
                }

                // Get token without @ sign
                std::string token{};
                Src->getSubStr(tokPos + 1, tokSize - 1, token);

                // Check if token is instruction
                if (isInstruction(token)) {
                    throwError("Instruction keyword inside label definition",
                               tokPos);
                    return false;
                } else if (isTypeInfo(token)) {
                    throwError("Type keyword inside label definition", tokPos);
                    return false;
                } else if (isRegister(token)) {
                    throwError("Register keyword inside label definition",
                               tokPos);
                    return false;
                } else {
                    // If the token is not an instruciton, type info or register
                    // then it must be an identifier
                    addToken(TokenType::LABEL_DEF, Cursor, CursorLineRow,
                             CursorLineColumn, 1);
                }
                eof = eatChar(c);
            } break;
            case '\n': {
                // Only add EOL tokens once in a row
                Token last = Tokens->back();
                if (last.Type != TokenType::EOL) {
                    addToken(TokenType::EOL, Cursor, CursorLineRow,
                             CursorLineColumn, 1);
                }
                incLineRow();
            }
                break;
            default:
                throwError("Unexpected character", Cursor);
                return false;
            }
            eof = eatChar(c);
        }
    }
    return true;
}
