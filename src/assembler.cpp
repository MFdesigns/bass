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
#include <filesystem>
#include <fstream>
#include <iostream>

Assembler::Assembler() {
    TokCursor = 0;
}

bool Assembler::readSource(char* pathName) {
    std::filesystem::path p{pathName};
    if (!std::filesystem::exists(p)) {
        std::cout << "[ERROR] Source file '" << pathName
                  << "' does not exist\n";
        return false;
    }

    uint32_t size = 0;
    // TODO: Add error checking
    // Get source file size
    std::ifstream stream{p};
    stream.seekg(0, std::ios::end);
    size = stream.tellg();
    stream.seekg(0, std::ios::beg);

    // Write complete file into buffer
    uint8_t* buffer = new uint8_t[size];
    stream.read((char*)buffer, size);

    Src = new Source(buffer, size);

    return true;
}

bool Assembler::assemble() {
    // Initialize components
    Scan = new Scanner{Src, &Tokens};

    bool scanSucc = Scan->scanSource();
    if (!scanSucc) {
        return false;
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
        uint8_t* data = Src->getData();
        std::string regName{(char*)&data[t.Index], t.Size};
        uint8_t regId = getRegisterTypeFromName(regName);
        RegisterId* reg =
            new RegisterId(t.Index, t.LineRow, t.LineCol, regId);
        base = reg;
        TokCursor++;
        t = Tokens[TokCursor];
    } else {
        std::cout << "Error: expected register in register offset\n";
        return;
    }

    if (t.Type == TokenType::RIGHT_SQUARE_BRACKET) {
        RegisterOffset* regOff =
            new RegisterOffset(t.Index, t.LineRow, t.LineCol,
                               RegisterLayout::REG, base, offset, intNum);
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
            uint8_t* data = Src->getData();
            std::string name{(char*)&data[t.Index], t.Size};
            uint64_t num = std::atoll(name.c_str());
            intNum = new IntegerNumber(t.Index, t.LineRow, t.LineCol, num);
            RegisterLayout layout = RegisterLayout::REG;
            // TODO: support different int sizes
            if (positive) {
                layout = RegisterLayout::REG_P_I32;
            } else {
                layout = RegisterLayout::REG_M_I32;
            }
            RegisterOffset* regOff = new RegisterOffset(
                t.Index, t.LineRow, t.LineCol, layout, base, offset, intNum);
            (*instr).Parameters.push_back(regOff);
            TokCursor++;
        } else {
            std::cout << "Error: expected ] bracket\n";
            return;
        }
    } else if (t.Type == TokenType::REGISTER_DEFINITION) {
        uint8_t* data = Src->getData();
        std::string regName{(char*)&data[t.Index], t.Size};
        uint8_t regId = getRegisterTypeFromName(regName);
        RegisterId* reg =
            new RegisterId(t.Index, t.LineRow, t.LineCol, regId);
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

        uint8_t* nameData = Src->getData();
        std::string name{(char*)&data[t.Index], t.Size};
        uint64_t num = std::atoll(name.c_str());
        intNum = new IntegerNumber(t.Index, t.LineRow, t.LineCol, num);
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
            RegisterOffset* regOff = new RegisterOffset(
                t.Index, t.LineRow, t.LineCol, layout, base, offset, intNum);
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
                    uint8_t* data = Src->getData();
                    std::string name{(char*)&data[id.Index], id.Size};
                    function =
                        new FuncDef(id.Index, id.LineRow, id.LineCol, name);
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
                uint8_t* data = Src->getData();
                std::string name{(char*)&data[t.Index], t.Size};
                instr = new Instruction(t.Index, t.LineRow, t.LineCol, name);
                (*function).Body.push_back(instr);

                // Check if next token terminates instruction and only set state
                // to parse its arguments if there are any
                Token peek = Tokens[TokCursor + 1];
                if (peek.Type != TokenType::INSTRUCTION &&
                    peek.Type != TokenType::LABEL_DEF &&
                    peek.Type != TokenType::RIGHT_CURLY_BRACKET) {
                    state = ASTState::INSTR_BODY;
                }
                TokCursor++;
            } else if (t.Type == TokenType::LABEL_DEF) {
                Token id = Tokens[TokCursor + 1];
                if (id.Type == TokenType::IDENTIFIER) {
                    uint8_t* data = Src->getData();
                    std::string name{(char*)&data[id.Index], id.Size};
                    LabelDef* labelDef =
                        new LabelDef(t.Index, t.LineRow, t.LineCol, name);
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
                uint8_t* data = Src->getData();
                std::string name{(char*)&data[t.Index], t.Size};
                UVMType type = getUVMTypeFromName(name);
                TypeInfo* typeInfo =
                    new TypeInfo(t.Index, t.LineRow, t.LineCol, type);
                (*instr).Parameters.push_back(typeInfo);
                TokCursor++;
                t = Tokens[TokCursor];

                if (t.Type == TokenType::INSTRUCTION ||
                    t.Type == TokenType::LABEL_DEF ||
                    t.Type == TokenType::RIGHT_CURLY_BRACKET) {
                    state = ASTState::FUNC_BODY;
                    continue;
                }
            }

            bool end = false;
            while (!end) {
                switch (t.Type) {
                case TokenType::IDENTIFIER: {
                    uint8_t* data = Src->getData();
                    std::string name{(char*)&data[t.Index], t.Size};
                    Identifier* id =
                        new Identifier(t.Index, t.LineRow, t.LineCol, name);
                    (*instr).Parameters.push_back(id);
                    TokCursor++;
                } break;
                case TokenType::REGISTER_DEFINITION: {
                    uint8_t* data = Src->getData();
                    std::string regName{(char*)&data[t.Index], t.Size};
                    uint8_t regId = getRegisterTypeFromName(regName);
                    RegisterId* reg =
                        new RegisterId(t.Index, t.LineRow, t.LineCol, regId);
                    (*instr).Parameters.push_back(reg);
                    TokCursor++;
                } break;
                case TokenType::LEFT_SQUARE_BRACKET:
                    parseRegOffset(instr);
                    TokCursor++;
                    break;
                case TokenType::INTEGER_NUMBER: {
                    uint8_t* data = Src->getData();
                    std::string name{(char*)&data[t.Index], t.Size};
                    uint64_t num = std::atoll(name.c_str());
                    IntegerNumber* id = new IntegerNumber(t.Index, t.LineRow,
                                                          t.LineCol, num);
                    (*instr).Parameters.push_back(id);
                    TokCursor++;
                } break;
                case TokenType::FLOAT_NUMBER: {
                    uint8_t* data = Src->getData();
                    std::string name{(char*)&data[t.Index], t.Size};
                    double num = std::atof(name.c_str());
                    FloatNumber* id =
                        new FloatNumber(t.Index, t.LineRow, t.LineCol, num);
                    (*instr).Parameters.push_back(id);
                    TokCursor++;
                } break;
                default:
                    std::cout << "Error: Expected parameter\n";
                    return false;
                    break;
                }
                t = Tokens[TokCursor];

                if (t.Type == TokenType::INSTRUCTION ||
                    t.Type == TokenType::LABEL_DEF ||
                    t.Type == TokenType::RIGHT_CURLY_BRACKET) {
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
