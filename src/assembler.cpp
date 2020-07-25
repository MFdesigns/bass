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

#include "assembler.h"
#include <iostream>

Assembler::Assembler(uint8_t* source, uint32_t sourceSize)
    : Source(source), SourceSize(sourceSize) {
    Cursor = 0;
    CursorLineRow = 1;
    CursorLineColumn = 1;
    TokCursor = 0;
}

void Assembler::incLineRow() {
    CursorLineRow++;
    CursorLineColumn = 0;
};

bool Assembler::eatChar(uint8_t& out) {
    bool eof = false;
    if (Cursor + 1 >= SourceSize) {
        eof = true;
    } else {
        Cursor++;
        CursorLineColumn++;
        out = Source[Cursor];
    }
    return eof;
}

bool Assembler::eatChars(uint32_t count, uint8_t& out) {
    bool eof = false;
    if (Cursor + count >= SourceSize) {
        eof = true;
    } else {
        Cursor += count;
        CursorLineColumn += count;
        out = Source[Cursor];
    }
    return eof;
}

bool Assembler::peekChar(uint8_t& out) {
    bool eof = false;
    if (Cursor + 1 >= SourceSize) {
        eof = true;
    } else {
        out = Source[Cursor + 1];
    }
    return eof;
}

void Assembler::addToken(TokenType type,
                         uint32_t index,
                         uint32_t lineRow,
                         uint32_t lineColumn,
                         uint32_t size) {
    Tokens.push_back(Token{type, index, size, lineRow, lineColumn});
}

bool Assembler::isInstruction(std::string& token) {
    bool isInstr = false;
    for (const auto& instr : INSTRUCTIONS) {
        if (token == instr) {
            isInstr = true;
            break;
        }
    }
    return isInstr;
}

bool Assembler::isTypeInfo(std::string& token) {
    bool isTypeInfo = false;
    for (const auto& typeInfo : TYPE_INFOS) {
        if (token == typeInfo) {
            isTypeInfo = true;
            break;
        }
    }
    return isTypeInfo;
}

bool Assembler::isRegister(std::string& token) {
    bool isRegister = false;
    for (const auto& reg : REGISTERS) {
        if (token == reg) {
            isRegister = true;
            break;
        }
    }
    return isRegister;
}

void Assembler::tokenizerError(const char* msg) {
    std::cout << "[Syntax Error] " << msg << " at Ln " << CursorLineRow
              << ", Col " << CursorLineColumn << " at char '" << Source[Cursor]
              << "' (" << (uint16_t)Source[Cursor] << ")\n";
}

bool Assembler::tokenize() {
    uint8_t c = Source[Cursor]; // char at current cursor position
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
            std::string token; // Temporary token builder string
            bool terminated = false;
            do {
                if (c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z' || c == '_' ||
                    c >= '0' && c <= '9') {
                    token.push_back(c);
                    // check if next char terminates identifier if not eat next
                    // char
                    uint8_t peek;
                    terminated = peekChar(peek);
                    if (!terminated &&
                        (peek == '\t' || peek == ' ' || peek == '{' ||
                         peek == '\n' || peek == ',' || peek == ']')) {
                        terminated = true;
                    } else if (!terminated) {
                        eatChar(c);
                    }
                } else {
                    tokenizerError("Invalid character in identifier");
                    return false;
                }
            } while (!terminated);

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
                        tokenizerError("Expected hex number");
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
                        tokenizerError("More than one decimal point");
                        return false;
                    } else {
                        tokenizerError("Expected number");
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
                    tokenizerError("Unexpected token");
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
            case '@':
                addToken(TokenType::AT_SIGN, Cursor, CursorLineRow,
                         CursorLineColumn, 1);
                break;
            case '\n':
                incLineRow();
                break;
            default:
                tokenizerError("Unexpected character");
                return false;
            }
            eof = eatChar(c);
        }
    }
    return true;
}

void Assembler::parseRegOffset(Instruction* instr) {
    RegisterId* base = nullptr;
    RegisterId* offset = nullptr;
    IntegerNumber* intNum = nullptr;
    bool positive = false;

    Token t = Tokens[TokCursor += 1];
    if (t.Type == TokenType::REGISTER_DEFINITION) {
        std::string regName{(char*)&Source[t.Index], t.Size};
        uint8_t regId = getRegisterTypeFromName(regName);
        RegisterId* reg = new RegisterId(t.Index, t.LineRow, t.LineColumn, regId);
        base = reg;
        TokCursor++;
        t = Tokens[TokCursor];
    } else {
        std::cout << "Error: expected register in register offset\n";
        return;
    }

    if (t.Type == TokenType::RIGHT_SQUARE_BRACKET) {
        RegisterOffset* regOff = new RegisterOffset(t.Index, t.LineRow, t.LineColumn, RegisterLayout::REG, base, offset, intNum);
        (*instr).Parameters.push_back(regOff);
        return;
    } else if (t.Type == TokenType::PLUS_SIGN) {
        positive = true;
    } else if (t.Type == TokenType::MINUS_SIGN) {
        positive = false;
    } else {
        std::cout << "Error: unexpected token in register offset\n";
        return;
    }
    t = Tokens[TokCursor += 1];

    if (t.Type == TokenType::INTEGER_NUMBER) {
        Token peek = Tokens[TokCursor + 1];
        if (peek.Type == TokenType::RIGHT_SQUARE_BRACKET) {
            std::string name{(char*)&Source[t.Index], t.Size};
            uint64_t num = std::atoll(name.c_str());
            intNum = new IntegerNumber(t.Index, t.LineRow, t.LineColumn, num);
            RegisterLayout layout = RegisterLayout::REG;
            // TODO: support different int sizes
            if (positive) {
                layout = RegisterLayout::REG_P_I32;
            } else {
                layout = RegisterLayout::REG_M_I32;
            }
            RegisterOffset* regOff = new RegisterOffset(t.Index, t.LineRow, t.LineColumn, layout, base, offset, intNum);
            (*instr).Parameters.push_back(regOff);
            TokCursor++;
        } else {
            std::cout << "Error: expected ] bracket\n";
            return;
        }
    } else if (t.Type == TokenType::REGISTER_DEFINITION) {
        std::string regName{(char*)&Source[t.Index], t.Size};
        uint8_t regId = getRegisterTypeFromName(regName);
        RegisterId* reg = new RegisterId(t.Index, t.LineRow, t.LineColumn, regId);
        offset = reg;
        TokCursor++;
        t = Tokens[TokCursor];

        if (t.Type == TokenType::ASTERISK) {
            TokCursor++;
            t = Tokens[TokCursor];
        } else {
            std::cout << "Error: expected * after offset\n";
            return;
        }

        std::string name{(char*)&Source[t.Index], t.Size};
        uint64_t num = std::atoll(name.c_str());
        intNum = new IntegerNumber(t.Index, t.LineRow, t.LineColumn, num);
        TokCursor++;
        t = Tokens[TokCursor];

        RegisterLayout layout = RegisterLayout::REG;
        if (t.Type == TokenType::RIGHT_SQUARE_BRACKET) {
            // TODO: support different int sizes
            if (positive) {
                layout = RegisterLayout::REG_P_REG_T_I16;
            } else {
                layout = RegisterLayout::REG_M_REG_T_I16;
            }
            RegisterOffset* regOff = new RegisterOffset(t.Index, t.LineRow, t.LineColumn, layout, base, offset, intNum);
            (*instr).Parameters.push_back(regOff);
        } else {
            std::cout << "Error: expected closing bracket after factor\n";
            return;
        }

    } else {
        std::cout << "Error: expected register or int number as offset\n";
        return;
    }
}

bool Assembler::buildAST() {
    ASTState state = ASTState::GLOBAL_SCOPE;
    FuncDef* function = nullptr;
    Instruction* instr = nullptr;

    while (TokCursor < Tokens.size()) {
        Token t = Tokens[TokCursor];
        switch (state) {
        case ASTState::GLOBAL_SCOPE: {
            Token id = Tokens[TokCursor];
            if (id.Type == TokenType::IDENTIFIER) {
                if (Tokens[TokCursor + 1].Type ==
                    TokenType::LEFT_CURLY_BRACKET) {
                    std::string name{(char*)&Source[id.Index], id.Size};
                    function =
                        new FuncDef(id.Index, id.LineRow, id.LineColumn, name);
                    AST.push_back(function);
                    state = ASTState::FUNC_BODY;
                    TokCursor += 2;
                } else {
                    std::cout << "Error: expected open curly bracket after "
                                 "function identifier\n";
                    return false;
                }
            } else {
                std::cout << "Error: expected function identifier\n";
                return false;
            }
        } break;
        case ASTState::FUNC_BODY: {
            if (t.Type == TokenType::INSTRUCTION) {
                std::string name{(char*)&Source[t.Index], t.Size};
                instr = new Instruction(t.Index, t.LineRow, t.LineColumn, name);
                (*function).Body.push_back(instr);

                // Check if next token terminates instruction and only set state to
                // parse its arguments if there are any
                Token peek = Tokens[TokCursor + 1];
                if (peek.Type != TokenType::INSTRUCTION && peek.Type != TokenType::AT_SIGN && peek.Type != TokenType::RIGHT_CURLY_BRACKET) {
                    state = ASTState::INSTR_BODY;
                }
                TokCursor++;
            } else if (t.Type == TokenType::AT_SIGN) {
                Token id = Tokens[TokCursor + 1];
                if (id.Type == TokenType::IDENTIFIER) {
                    std::string name{(char*)&Source[id.Index], id.Size};
                    LabelDef* labelDef =
                        new LabelDef(t.Index, t.LineRow, t.LineColumn, name);
                    (*function).Body.push_back(labelDef);
                    TokCursor += 2;
                } else {
                    std::cout << "Expected identifier after label keyword\n";
                    return false;
                }
            } else if (t.Type == TokenType::RIGHT_CURLY_BRACKET) {
                state = ASTState::GLOBAL_SCOPE;
                TokCursor++;
            } else {
                std::cout << "Error: expected instruction or label after "
                             "inside function body\n";
                return false;
            }
        } break;
        case ASTState::INSTR_BODY: {
            if (t.Type == TokenType::TYPE_INFO) {
                std::string name{(char*)&Source[t.Index], t.Size};
                UVMType type = getUVMTypeFromName(name);
                TypeInfo* typeInfo = new TypeInfo(t.Index, t.LineRow, t.LineColumn, type);
                (*instr).Parameters.push_back(typeInfo);
                TokCursor++;
                t = Tokens[TokCursor];

                if (t.Type == TokenType::INSTRUCTION || t.Type == TokenType::AT_SIGN || t.Type == TokenType::RIGHT_CURLY_BRACKET) {
                    state = ASTState::FUNC_BODY;
                    continue;
                }
            }

            bool end = false;
            while (!end) {
                switch (t.Type) {
                    case TokenType::IDENTIFIER: {
                        std::string name{(char*)&Source[t.Index], t.Size};
                        Identifier* id = new Identifier(t.Index, t.LineRow, t.LineColumn, name);
                        (*instr).Parameters.push_back(id);
                        TokCursor++;
                    }
                    break;
                    case TokenType::REGISTER_DEFINITION: {
                        std::string regName{(char*)&Source[t.Index], t.Size};
                        uint8_t regId = getRegisterTypeFromName(regName);
                        RegisterId* reg = new RegisterId(t.Index, t.LineRow, t.LineColumn, regId);
                        (*instr).Parameters.push_back(reg);
                        TokCursor++;
                    }
                    break;
                    case TokenType::LEFT_SQUARE_BRACKET:
                        parseRegOffset(instr);
                        TokCursor++;
                    break;
                    case TokenType::INTEGER_NUMBER: {
                        std::string name{(char*)&Source[t.Index], t.Size};
                        uint64_t num = std::atoll(name.c_str());
                        IntegerNumber* id = new IntegerNumber(t.Index, t.LineRow, t.LineColumn, num);
                        (*instr).Parameters.push_back(id);
                        TokCursor++;
                    }
                    break;
                    case TokenType::FLOAT_NUMBER: {
                        std::string name{(char*)&Source[t.Index], t.Size};
                        double num = std::atof(name.c_str());
                        FloatNumber* id = new FloatNumber(t.Index, t.LineRow, t.LineColumn, num);
                        (*instr).Parameters.push_back(id);
                        TokCursor++;
                    }
                    break;
                    default:
                        std::cout << "Error: Expected parameter\n";
                        return false;
                    break;
                }
                t = Tokens[TokCursor];

                if (t.Type == TokenType::INSTRUCTION || t.Type == TokenType::AT_SIGN || t.Type == TokenType::RIGHT_CURLY_BRACKET)  {
                    state = ASTState::FUNC_BODY;
                    end = true;
                } else if (t.Type == TokenType::COMMA) {
                    TokCursor++;
                    t = Tokens[TokCursor];
                }
            }
        } break;
        }
    }

    return true;
}
