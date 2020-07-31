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

#include "parser.hpp"
#include <iomanip>
#include <iostream>

Parser::Parser(Source* src, std::vector<Token>* tokens, Global* global)
    : Src(src), Tokens(tokens), Glob(global){};

UVMType Parser::getUVMType(Token* tok) {
    std::string typeName;
    Src->getSubStr(tok->Index, tok->Size, typeName);

    UVMType type = UVMType::I8;
    if (typeName == "i8") {
        type = UVMType::I8;
    } else if (typeName == "i16") {
        type = UVMType::I16;
    } else if (typeName == "i32") {
        type = UVMType::I32;
    } else if (typeName == "i64") {
        type = UVMType::I64;
    } else if (typeName == "f32") {
        type = UVMType::F32;
    } else if (typeName == "f64") {
        type = UVMType::F64;
    }
    return type;
}

int64_t Parser::strToInt(std::string& str) {
    int64_t num = 0;
    int32_t base = 10;
    if (str.size() >= 3) {
        if (str[0] == '0' && str[1] == 'x') {
            base = 16;
        }
    }
    num = std::stoll(str, 0, base);
    return num;
}

uint8_t Parser::getRegisterTypeFromName(std::string& regName) {
    uint8_t id = 0;
    if (regName == "ip") {
        id = 1;
    } else if (regName == "bp") {
        id = 3;
    } else if (regName == "sp") {
        id = 2;
    } else {
        uint8_t base = 0;
        if (regName[0] == 'r') {
            base = 0x5;
        } else if (regName[0] == 'f') {
            base = 0x16;
        }

        const char* num = regName.c_str();
        uint32_t offset = std::atoi(&num[1]);
        id = base + offset;
    }
    return id;
}

Token* Parser::eatToken() {
    Token* tok = nullptr;
    if (Cursor < Tokens->size()) {
        // Because Cursor starts at index 0 return current token before
        // increasing the cursor
        tok = &(*Tokens)[Cursor];
        Cursor++;
    } else {
        tok = &(*Tokens)[Tokens->size() - 1];
    }
    return tok;
}

bool Parser::peekToken(Token** tok) {
    if (Cursor < Tokens->size()) {
        *tok = &(*Tokens)[Cursor];
        return true;
    }
    return false;
}

void Parser::throwError(const char* msg, Token& tok) {
    std::string line;
    uint32_t lineIndex = 0;
    Src->getLine(tok.Index, line, lineIndex);
    std::string lineNr = std::to_string(lineIndex);

    uint32_t errOffset = tok.Index - lineIndex;

    std::cout << "[Parser Error] " << msg << " at Ln " << tok.LineRow
              << ", Col " << tok.LineCol << '\n'
              << "  " << tok.LineRow << " | " << line << '\n'
              << std::setw(2 + lineNr.size()) << std::setfill(' ') << " |"
              << std::setw(errOffset + 1) << std::setfill(' ') << ' '
              << std::setw(tok.Size) << std::setfill('~') << '~' << '\n';
}

bool Parser::parseRegOffset(Instruction* instr) {
    RegisterId* base = nullptr;
    RegisterId* offset = nullptr;
    IntegerNumber* intNum = nullptr;
    bool positive = false;

    Token* t = eatToken();
    if (t->Type == TokenType::REGISTER_DEFINITION) {
        std::string regName;
        Src->getSubStr(t->Index, t->Size, regName);
        uint8_t regId = getRegisterTypeFromName(regName);
        base = new RegisterId(t->Index, t->LineRow, t->LineCol, regId);
        t = eatToken();
    } else {
        throwError("Expected register in register offset", *t);
        return false;
    }

    if (t->Type == TokenType::RIGHT_SQUARE_BRACKET) {
        RegisterOffset* regOff =
            new RegisterOffset(t->Index, t->LineRow, t->LineCol,
                               RegisterLayout::REG, base, offset, intNum);
        instr->Params.push_back(regOff);
        return true;
    } else if (t->Type == TokenType::PLUS_SIGN) {
        positive = true;
    } else if (t->Type == TokenType::MINUS_SIGN) {
        positive = false;
    } else {
        throwError("Unexpected token in register offset", *t);
        return false;
    }
    t = eatToken();

    if (t->Type == TokenType::INTEGER_NUMBER) {
        Token* peek;
        bool eof = !peekToken(&peek);
        if (!eof && peek->Type == TokenType::RIGHT_SQUARE_BRACKET) {
            std::string numStr;
            Src->getSubStr(t->Index, t->Size, numStr);
            int64_t num = strToInt(numStr);
            intNum = new IntegerNumber(t->Index, t->LineRow, t->LineCol, num);
            RegisterLayout layout = RegisterLayout::REG;
            // TODO: support different int sizes
            if (positive) {
                layout = RegisterLayout::REG_P_I32;
            } else {
                layout = RegisterLayout::REG_M_I32;
            }
            RegisterOffset* regOff = new RegisterOffset(
                t->Index, t->LineRow, t->LineCol, layout, base, offset, intNum);
            instr->Params.push_back(regOff);
            t = eatToken();
        } else {
            throwError("Expected closing bracket ]", *t);
            return false;
        }
    } else if (t->Type == TokenType::REGISTER_DEFINITION) {
        std::string regName;
        Src->getSubStr(t->Index, t->Size, regName);
        uint8_t regId = getRegisterTypeFromName(regName);
        offset = new RegisterId(t->Index, t->LineRow, t->LineCol, regId);
        t = eatToken();
        if (t->Type == TokenType::ASTERISK) {
            t = eatToken();
        } else {
            throwError("Expected * after offset", *t);
            return false;
        }

        std::string numStr;
        Src->getSubStr(t->Index, t->Size, numStr);
        int64_t num = strToInt(numStr);
        intNum = new IntegerNumber(t->Index, t->LineRow, t->LineCol, num);
        t = eatToken();

        RegisterLayout layout = RegisterLayout::REG;
        if (t->Type == TokenType::RIGHT_SQUARE_BRACKET) {
            // TODO: support different int sizes
            if (positive) {
                layout = RegisterLayout::REG_P_REG_T_I16;
            } else {
                layout = RegisterLayout::REG_M_REG_T_I16;
            }
            RegisterOffset* regOff = new RegisterOffset(
                t->Index, t->LineRow, t->LineCol, layout, base, offset, intNum);
            instr->Params.push_back(regOff);
        } else {
            throwError("Expectd closing bracket after factor", *t);
            return false;
        }

    } else {
        throwError("Expected register or int number as offset", *t);
        return false;
    }
    return true;
}

bool Parser::buildAST() {
    ParseState state = ParseState::GLOBAL_SCOPE;
    // Pointer to currently parsed function or instruction
    FuncDef* func = nullptr;
    Instruction* instr = nullptr;

    while (state != ParseState::END) {
        Token* t = eatToken();
        switch (state) {
        case ParseState::GLOBAL_SCOPE: {
            // Ignore new line token
            if (t->Type == TokenType::EOL) {
                t = eatToken();
            }

            if (t->Type == TokenType::END_OF_FILE) {
                state = ParseState::END;
                continue;
            }

            // If the current state is the Global scope we expect a identifier
            // followed by an open curly bracket
            if (t->Type != TokenType::IDENTIFIER) {
                throwError("Expected identifier", *t);
                return false;
            }

            std::string funcName;
            Src->getSubStr(t->Index, t->Size, funcName);
            Token* peek = nullptr;
            bool eof = !peekToken(&peek);
            if (eof || peek->Type != TokenType::LEFT_CURLY_BRACKET) {
                throwError("Expected { in function definition", *t);
                return false;
            }

            t = eatToken();
            func = new FuncDef(t->Index, t->LineRow, t->LineCol, funcName);
            Glob->Body.push_back(func);
            state = ParseState::FUNC_BODY;
        } break;
        case ParseState::FUNC_BODY: {
            // Skip new line token
            if (t->Type == TokenType::EOL) {
                t = eatToken();
                if (t->Type == TokenType::END_OF_FILE) {
                    throwError("Unexpected end of file in function body", *t);
                    return false;
                }
            }

            if (t->Type == TokenType::RIGHT_CURLY_BRACKET) {
                state = ParseState::GLOBAL_SCOPE;
                continue;
            }

            switch (t->Type) {
            case TokenType::INSTRUCTION: {
                std::string instrName;
                Src->getSubStr(t->Index, t->Size, instrName);
                instr = new Instruction(t->Index, t->LineRow, t->LineCol,
                                        instrName);
                func->Body.push_back(instr);

                Token* peek = nullptr;
                bool eof = !peekToken(&peek);
                if (eof) {
                    throwError("Unexpected end of file after instruction", *t);
                    return false;
                }

                if (peek->Type != TokenType::EOL) {
                    state = ParseState::INSTR_BODY;
                }
            } break;
            case TokenType::LABEL_DEF: {
                std::string labelName;
                Src->getSubStr(t->Index, t->Size, labelName);
                LabelDef* label =
                    new LabelDef(t->Index, t->LineRow, t->LineCol, labelName);
                func->Body.push_back(label);

                Token* peek = nullptr;
                bool eof = !peekToken(&peek);
                if (eof || peek->Type != TokenType::EOL) {
                    throwError("Expected new line after label definition", *t);
                    return false;
                }
                t = eatToken(); // TODO: BUG ?
            } break;
            case TokenType::RIGHT_CURLY_BRACKET:
                state = ParseState::GLOBAL_SCOPE;
                continue;
                break;
            default:
                throwError("Unexpected token in function body", *t);
                return false;
                break;
            }
        } break;
        case ParseState::INSTR_BODY: {
            if (t->Type == TokenType::TYPE_INFO) {
                UVMType typeUVMType = getUVMType(t);
                TypeInfo* typeInfo =
                    new TypeInfo(t->Index, t->LineRow, t->LineCol, typeUVMType);
                instr->Params.push_back(typeInfo);
                t = eatToken();
            }

            bool endOfParamList = false;
            while (!endOfParamList) {
                switch (t->Type) {
                case TokenType::IDENTIFIER: {
                    std::string idName;
                    Src->getSubStr(t->Index, t->Size, idName);
                    Identifier* id = new Identifier(t->Index, t->LineRow,
                                                    t->LineCol, idName);
                    instr->Params.push_back(id);
                } break;
                case TokenType::REGISTER_DEFINITION: {
                    std::string regName;
                    Src->getSubStr(t->Index, t->Size, regName);
                    uint8_t regType = getRegisterTypeFromName(regName);
                    RegisterId* reg = new RegisterId(t->Index, t->LineRow,
                                                     t->LineCol, regType);
                    instr->Params.push_back(reg);
                } break;
                case TokenType::LEFT_SQUARE_BRACKET: {
                    bool validRO = parseRegOffset(instr);
                    if (!validRO) {
                        return false;
                    }
                } break;
                case TokenType::INTEGER_NUMBER: {
                    std::string numStr;
                    Src->getSubStr(t->Index, t->Size, numStr);
                    int64_t num = strToInt(numStr);
                    IntegerNumber* iNum = new IntegerNumber(
                        t->Index, t->LineRow, t->LineCol, num);
                    instr->Params.push_back(iNum);
                } break;
                case TokenType::FLOAT_NUMBER: {
                    std::string floatStr;
                    Src->getSubStr(t->Index, t->Size, floatStr);
                    double num = std::atof(floatStr.c_str());
                    FloatNumber* iNum =
                        new FloatNumber(t->Index, t->LineRow, t->LineCol, num);
                    instr->Params.push_back(iNum);
                } break;
                default:
                    throwError("Expected parameter", *t);
                    return false;
                    break;
                }
                t = eatToken();

                if (t->Type == TokenType::COMMA) {
                    t = eatToken();
                } else if (t->Type == TokenType::EOL) {
                    endOfParamList = true;
                }
            }
            state = ParseState::FUNC_BODY;
        } break;
        }
    }
    return true;
}
