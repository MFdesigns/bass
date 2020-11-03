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

#include "parser.hpp"
#include "asm/asm.hpp"
#include "asm/encoding.hpp"
#include <iomanip>
#include <iostream>

/**
 * Constructs a new Parser
 * @param instrDefs Pointer to instruction definitons
 * @param src Pointer to the source file
 * @param tokens Pointer to the token array
 * @param global [out] Pointer to the Global AST node
 * @param funcDefs [out] Pointer to the FuncDefLookup
 */
Parser::Parser(std::vector<InstrDefNode>* instrDefs,
               SourceFile* src,
               std::vector<Token>* tokens,
               Global* global,
               std::vector<LabelDefLookup>* funcDefs)
    : InstrDefs(instrDefs), Src(src), Tokens(tokens), Glob(global),
      LabelDefs(funcDefs){};

uint8_t Parser::getUVMType(Token* tok) {
    std::string typeName;
    Src->getSubStr(tok->Index, tok->Size, typeName);

    uint8_t type = 0;
    if (typeName == "i8") {
        type = UVM_TYPE_I8;
    } else if (typeName == "i16") {
        type = UVM_TYPE_I16;
    } else if (typeName == "i32") {
        type = UVM_TYPE_I32;
    } else if (typeName == "i64") {
        type = UVM_TYPE_I64;
    } else if (typeName == "f32") {
        type = UVM_TYPE_F32;
    } else if (typeName == "f64") {
        type = UVM_TYPE_F64;
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
    // Warning: this only works with unsigned numbers and  does not handle
    // sigend numbers
    num = std::stoull(str, 0, base);
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
    constexpr uint8_t RO_LAYOUT_NEG = 0b1000'0000;
    constexpr uint8_t RO_LAYOUT_POS = 0b0000'0000;
    RegisterOffset* regOff = new RegisterOffset();
    Token* t = eatToken();

    if (t->Type == TokenType::REGISTER_DEFINITION) {
        std::string regName;
        Src->getSubStr(t->Index, t->Size, regName);
        uint8_t regId = getRegisterTypeFromName(regName);
        regOff->Base = new RegisterId(t->Index, t->LineRow, t->LineCol, regId);
        t = eatToken();
    } else {
        throwError("Expected register in register offset", *t);
        return false;
    }

    if (t->Type == TokenType::RIGHT_SQUARE_BRACKET) {
        regOff->Position = t->Index;
        regOff->LineNumber = t->LineRow;
        regOff->LineColumn = t->LineCol;
        regOff->Layout = RO_LAYOUT_IR;
        instr->Params.push_back(regOff);
        return true;
    } else if (t->Type == TokenType::PLUS_SIGN) {
        regOff->Layout |= RO_LAYOUT_POS;
    } else if (t->Type == TokenType::MINUS_SIGN) {
        regOff->Layout |= RO_LAYOUT_NEG;
    } else {
        throwError("Unexpected token in register offset", *t);
        return false;
    }

    t = eatToken();
    // <iR> +/- <i32>
    if (t->Type == TokenType::INTEGER_NUMBER) {
        Token* peek;
        bool eof = !peekToken(&peek);
        if (!eof && peek->Type == TokenType::RIGHT_SQUARE_BRACKET) {
            // Get int string and convert to an int
            std::string numStr;
            Src->getSubStr(t->Index, t->Size, numStr);
            int64_t num = strToInt(numStr);

            // <iR> + <i32> expects integer to have a maximum size of 32 bits
            // Check if the requirement is meet otherwise throw error
            if (num >> 32 != 0) {
                throwError(
                    "Register offset immediate does not fit into 32-bit value",
                    *t);
                return false;
            }
            regOff->Immediate.U32 = (uint32_t)num;

            // TODO: Register offset position is not correct
            regOff->Position = t->Index;
            regOff->LineNumber = t->LineRow;
            regOff->LineColumn = t->LineCol;
            regOff->Layout |= RO_LAYOUT_IR_INT;
            instr->Params.push_back(regOff);
            t = eatToken();
        } else {
            throwError("Expected closing bracket after immediate offset inside "
                       "register offset ]",
                       *t);
            return false;
        }
    } else if (t->Type == TokenType::REGISTER_DEFINITION) {
        std::string regName;
        Src->getSubStr(t->Index, t->Size, regName);
        uint8_t regIdType = getRegisterTypeFromName(regName);
        regOff->Offset =
            new RegisterId(t->Index, t->LineRow, t->LineCol, regIdType);
        t = eatToken();
        if (t->Type == TokenType::ASTERISK) {
            t = eatToken();
        } else {
            throwError("Expected * after offset inside register offset", *t);
            return false;
        }

        std::string numStr;
        Src->getSubStr(t->Index, t->Size, numStr);
        int64_t num = strToInt(numStr);

        // <iR> +/- <iR> * <i16> expects integer to have a maximum size of 16
        // bits Check if the requirement is meet otherwise throw error
        if (num >> 16 != 0) {
            throwError(
                "Register offset immediate does not fit into 16-bit value", *t);
            return false;
        }
        regOff->Immediate.U16 = (uint16_t)num;
        t = eatToken();

        if (t->Type == TokenType::RIGHT_SQUARE_BRACKET) {
            // TODO: Register offset position is not correct
            regOff->Position = t->Index;
            regOff->LineNumber = t->LineRow;
            regOff->LineColumn = t->LineCol;
            regOff->Layout |= RO_LAYOUT_IR_IR_INT;
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
    Instruction* instr = nullptr;

    while (state != ParseState::END) {
        Token* t = eatToken();
        switch (state) {
        case ParseState::GLOBAL_SCOPE: {
            // Skip new line token
            if (t->Type == TokenType::EOL) {
                t = eatToken();
            }

            if (t->Type == TokenType::END_OF_FILE) {
                state = ParseState::END;
                continue;
            }

            switch (t->Type) {
            case TokenType::INSTRUCTION: {
                std::string instrName;
                Src->getSubStr(t->Index, t->Size, instrName);
                instr = new Instruction(t->Index, t->LineRow, t->LineCol,
                                        instrName, t->Tag);
                Glob->Body.push_back(instr);

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
                // + 1 because @ sign at start of label should be ignored
                Src->getSubStr(t->Index + 1, t->Size - 1, labelName);
                LabelDef* label =
                    new LabelDef(t->Index, t->LineRow, t->LineCol, labelName);
                Glob->Body.push_back(label);

                Token* peek = nullptr;
                bool eof = !peekToken(&peek);
                if (eof || peek->Type != TokenType::EOL) {
                    throwError("Expected new line after label definition", *t);
                    return false;
                }
                t = eatToken(); // TODO: BUG ?
            } break;
            default:
                throwError("Unexpected token in function body", *t);
                return false;
                break;
            }
        } break;
        case ParseState::INSTR_BODY: {
            if (t->Type == TokenType::TYPE_INFO) {
                uint8_t typeUVMType = getUVMType(t);
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
            state = ParseState::GLOBAL_SCOPE;
        } break;
        }
    }
    return true;
}

/**
 * Checks if instruction has valid parameters
 * @param instr Pointer to Instruction to type check
 * @param labelRefs Reference to array of label referenced
 * @param funcRefs Reference to array of function referenced
 * @return On success returns true otherwise false
 */
bool Parser::typeCheckInstrParams(Instruction* instr,
                                  std::vector<Identifier*>& labelRefs) {
    // Get top node of instruction paramters tree
    InstrDefNode* paramNode = &(*InstrDefs)[instr->ASMDefIndex];

    // Check if instr has no parameters and see if definiton accpets no
    // paramters. Important: this assumes that an instruction definiton either
    // has 0 parameters or only paramter definitons with at least 1. It cannot
    // have both
    if (instr->Params.size() == 0) {
        if (paramNode->Children.size() == 0) {
            // Attach opcode and return
            instr->Opcode = paramNode->ParamList->Opcode;
            instr->EncodingFlags = paramNode->ParamList->Flags;
            return true;
        } else {
            std::cout
                << "[Type Checker] Error: expected paramters found none\n";
            return false;
        }
    }

    // Try to find instruction parameter signature
    InstrParamList* paramList = nullptr;
    InstrDefNode* currentNode = paramNode;
    // This is a reference used to tag every float/int paramter with the correct
    // type and select the corrent opcode variant. This assumes that there can
    // only ever be one TypeInfo in the paramters of an instruction.
    TypeInfo* type = nullptr;
    for (uint32_t i = 0; i < instr->Params.size(); i++) {
        InstrDefNode* nextNode = nullptr;
        for (uint32_t n = 0; n < currentNode->Children.size(); n++) {
            ASTNode* astNode = instr->Params[i];
            switch (currentNode->Children[n].Type) {
            case InstrParamType::INT_TYPE: {
                if (astNode->Type != ASTType::TYPE_INFO) {
                    break;
                }
                TypeInfo* typeInfo = dynamic_cast<TypeInfo*>(astNode);
                if (typeInfo->DataType != UVM_TYPE_I8 &&
                    typeInfo->DataType != UVM_TYPE_I16 &&
                    typeInfo->DataType != UVM_TYPE_I32 &&
                    typeInfo->DataType != UVM_TYPE_I64) {
                    std::cout << "[Type Checker] Error: Expected int type "
                                 "found float type\n";
                    break;
                }
                type = typeInfo;
                nextNode = &currentNode->Children[n];
            } break;
            case InstrParamType::FLOAT_TYPE: {
                if (astNode->Type != ASTType::TYPE_INFO) {
                    break;
                }
                TypeInfo* typeInfo = dynamic_cast<TypeInfo*>(astNode);
                if (typeInfo->DataType != UVM_TYPE_F32 &&
                    typeInfo->DataType != UVM_TYPE_F64) {
                    std::cout << "[Type Checker] Error: Expected float type "
                                 "found int type\n";
                    break;
                }
                type = typeInfo;
                nextNode = &currentNode->Children[n];
            } break;
            case InstrParamType::LABEL_ID: {
                if (astNode->Type != ASTType::IDENTIFIER) {
                    break;
                }
                Identifier* labelRef = dynamic_cast<Identifier*>(astNode);
                labelRefs.push_back(labelRef);
                nextNode = &currentNode->Children[n];
            } break;
            case InstrParamType::INT_REG: {
                if (astNode->Type != ASTType::REGISTER_ID) {
                    break;
                }
                RegisterId* regId = dynamic_cast<RegisterId*>(astNode);
                // s: What about flag register ?
                if (regId->Id < 0x1 && regId->Id > 0x15) {
                    std::cout
                        << "[Type Checker] Error: Expected integer register\n";
                    break;
                }
                nextNode = &currentNode->Children[n];
            } break;
            case InstrParamType::FLOAT_REG: {
                if (astNode->Type != ASTType::REGISTER_ID) {
                    break;
                }
                RegisterId* regId = dynamic_cast<RegisterId*>(astNode);
                if (regId->Id < 0x16 && regId->Id > 0x26) {
                    std::cout
                        << "[Type Checker] Error: Expected integer register\n";
                    break;
                }
                nextNode = &currentNode->Children[n];
            } break;
            case InstrParamType::REG_OFFSET: {
                if (astNode->Type != ASTType::REGISTER_OFFSET) {
                    break;
                }
                nextNode = &currentNode->Children[n];
            } break;
            case InstrParamType::INT_NUM: {
                if (astNode->Type != ASTType::INTEGER_NUMBER) {
                    break;
                }
                IntegerNumber* num = dynamic_cast<IntegerNumber*>(astNode);
                num->DataType = type->DataType;
                nextNode = &currentNode->Children[n];
            } break;
            case InstrParamType::FLOAT_NUM: {
                if (astNode->Type != ASTType::FLOAT_NUMBER) {
                    break;
                }

                FloatNumber* num = dynamic_cast<FloatNumber*>(astNode);
                num->DataType = type->DataType;
                nextNode = &currentNode->Children[n];
            } break;
            case InstrParamType::SYS_INT: {
                if (astNode->Type != ASTType::INTEGER_NUMBER) {
                    break;
                }

                IntegerNumber* num = dynamic_cast<IntegerNumber*>(astNode);
                // syscall args are always 1 byte
                num->DataType = UVM_TYPE_I8;
                nextNode = &currentNode->Children[n];
            } break;
            }
        }

        if (nextNode == nullptr) {
            break;
        } else {
            currentNode = nextNode;
        }

        if (i + 1 == instr->Params.size()) {
            paramList = currentNode->ParamList;
        }
    }

    if (paramList == nullptr) {
        std::cout << "Error no matching parameter list found for instruction "
                  << instr->Name << " at Ln " << instr->LineNumber << " Col "
                  << instr->LineColumn << '\n';
        return false;
    }

    // Check if the opcode is determined by a type variant and attach the opcode
    // to the Instruction node
    if (paramList->Flags & INSTR_FLAG_TYPE_VARIANTS) {
        uint8_t opcode = 0;
        // Find opcode variant
        for (uint32_t i = 0; i < paramList->OpcodeVariants.size(); i++) {
            TypeVariant* variant = &paramList->OpcodeVariants[i];
            if (variant->Type == type->DataType) {
                opcode = variant->Opcode;
            }
        }
        instr->Opcode = opcode;
    } else {
        instr->Opcode = paramList->Opcode;
    }
    // Attach encoding information
    instr->EncodingFlags = paramList->Flags;

    return true;
}

/**
 * Performs a complete type checking pass over the AST
 * @return Returns true if no errors occured otherwise false
 */
bool Parser::typeCheck() {
    // Check if Global AST node has no children which means the main function is
    // missing for sure
    if (Glob->Body.size() == 0) {
        std::cout << "[Type Checker] Missing main label\n";
        return false;
    }

    // Try to find main entry point
    LabelDef* mainEntry = nullptr;
    for (uint32_t i = 0; i < Glob->Body.size(); i++) {
        ASTNode* node = Glob->Body[i];
        if (node->Type == ASTType::LABEL_DEFINITION) {
            LabelDef* label = dynamic_cast<LabelDef*>(node);
            if (label->Name == "main") {
                mainEntry = label;
            }
        }
    }

    // Check if main function was found
    if (mainEntry == nullptr) {
        std::cout << "[Type Checker] Missing main entry\n";
        return false;
    }

    // This is used to keep track of the localy referenced function and label
    // identifiers aswell as localy defined label definitions
    std::vector<Identifier*> labelRefs;

    // Tracks if an error occured while type checking
    bool typeCheckError = false;
    // Type check complete AST. This assumes that the build AST generated a
    // valid AST
    for (const auto& globElem : Glob->Body) {
        if (globElem->Type == ASTType::LABEL_DEFINITION) {
            LabelDef* label = dynamic_cast<LabelDef*>(globElem);

            // Check if function definition is redifined
            bool labelRedef = false;
            for (uint32_t i = 0; i < LabelDefs->size(); i++) {
                if ((*LabelDefs)[i].Def->Name == label->Name) {
                    labelRedef = true;
                    break;
                }
            }

            // If function is a redefinition continue with parsing the function
            // body anyway
            if (labelRedef) {
                typeCheckError = true;
                std::cout << "[Type Checker] Error: label is already defined\n";
            }
            LabelDefs->push_back(LabelDefLookup{label, 0});
        } else if (globElem->Type == ASTType::INSTRUCTION) {
            Instruction* instr = dynamic_cast<Instruction*>(globElem);
            if (!typeCheckInstrParams(instr, labelRefs)) {
                typeCheckError = true;
                continue;
            }
        }
    }

    // Check if all label references are resolved
    for (const auto& labelRef : labelRefs) {
        bool foundDef = false;
        for (uint32_t i = 0; i < LabelDefs->size(); i++) {
            if (labelRef->Name == (*LabelDefs)[i].Def->Name) {
                foundDef = true;
                break;
            }
        }

        if (!foundDef) {
            std::cout << "[Type Checker] Error: unresolved label '"
                      << labelRef->Name << "'\n";
            typeCheckError = true;
        }
    }

    return !typeCheckError;
}
