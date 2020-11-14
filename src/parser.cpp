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
#include "cli.hpp"
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
               const std::vector<Token>* tokens,
               ASTFileNode* fileNode,
               std::vector<LabelDefLookup>* funcDefs)
    : InstrDefs(instrDefs), Src(src), Tokens(tokens), FileNode(fileNode),
      LabelDefs(funcDefs){};

/**
 * Converts a string to an integer
 * @param str String to be converted
 * @return Integer of string
 */
int64_t Parser::strToInt(std::string& str) {
    int64_t num = 0;
    int32_t base = 10;
    if (str.size() >= 3) {
        if (str[0] == '0' && str[1] == 'x') {
            base = 16;
        }
    }
    // Warning: this only works with unsigned numbers and  does not handle
    // signed numbers
    num = std::stoull(str, 0, base);
    return num;
}

/**
 * Returns token at current Cursor and increases the Cursor
 * @return Pointer to current Token, if Cursor is at the end will always return
 * the last token
 */
Token* Parser::eatToken() {
    const Token* tok = nullptr;
    if (Cursor < Tokens->size()) {
        // Because Cursor starts at index 0 return current token before
        // increasing the cursor
        tok = &(*Tokens)[Cursor];
        Cursor++;
    } else {
        tok = &Tokens->back();
    }
    return const_cast<Token*>(tok);
}

/**
 * Return the next Token without increasing the Cursor
 * @return Pointer to current Token, if Cursor is at the end will always return
 * the last token
 */
Token* Parser::peekToken() {
    const Token* tok = nullptr;
    if (Cursor >= Tokens->size()) {
        tok = &Tokens->back();
    } else {
        tok = &(*Tokens)[Cursor];
    }
    return const_cast<Token*>(tok);
}

/**
 * Skips token input until new line
 */
void Parser::skipLine() {
    Token* tok = eatToken();
    while (tok->Type != TokenType::END_OF_FILE && tok->Type != TokenType::EOL) {
        tok = eatToken();
    }
}

/**
 * Prints an error to the console
 * @param msg Pointer to error message string
 * @param tok Token to be displayed
 */
void Parser::printTokenError(const char* msg, Token& tok) {
    printError(Src, tok.Index, tok.Size, tok.LineRow, tok.LineCol, msg);
}

/**
 * Parses a register offset and appends it to the parent instruction node
 * @param instr Pointer to parent instruction node
 * @return On valid register offset returns true otherwise false
 */
bool Parser::parseRegOffset(Instruction* instr) {
    constexpr uint8_t RO_LAYOUT_NEG = 0b1000'0000;
    constexpr uint8_t RO_LAYOUT_POS = 0b0000'0000;
    RegisterOffset* regOff = new RegisterOffset();
    Token* t = eatToken();

    // Check if register offset is a variable offset e.g "[staticVar]"
    if (t->Type == TokenType::IDENTIFIER) {
        std::string idString;
        Src->getSubStr(t->Index, t->Size, idString);
        regOff->Var =
            new Identifier(t->Index, t->Size, t->LineRow, t->LineCol, idString);

        t = eatToken();
        // Closing bracket
        if (t->Type != TokenType::RIGHT_SQUARE_BRACKET) {
            printTokenError(
                "Expected closing bracket ] after variable reference", *t);
            return false;
        }
        instr->Params.push_back(regOff);
        return true;
    }

    if (t->Type == TokenType::REGISTER_DEFINITION) {
        regOff->Base =
            new RegisterId(t->Index, t->Size, t->LineRow, t->LineCol, t->Tag);
        t = eatToken();
    } else {
        printTokenError("Expected register in register offset", *t);
        return false;
    }

    if (t->Type == TokenType::RIGHT_SQUARE_BRACKET) {
        regOff->Index = t->Index;
        regOff->LineRow = t->LineRow;
        regOff->LineCol = t->LineCol;
        regOff->Layout = RO_LAYOUT_IR;
        instr->Params.push_back(regOff);
        return true;
    } else if (t->Type == TokenType::PLUS_SIGN) {
        regOff->Layout |= RO_LAYOUT_POS;
    } else if (t->Type == TokenType::MINUS_SIGN) {
        regOff->Layout |= RO_LAYOUT_NEG;
    } else {
        printTokenError("Unexpected token in register offset", *t);
        return false;
    }

    t = eatToken();
    // <iR> +/- <i32>
    if (t->Type == TokenType::INTEGER_NUMBER) {
        Token* peek = peekToken();
        if (peek != nullptr && peek->Type == TokenType::RIGHT_SQUARE_BRACKET) {
            // Get int string and convert to an int
            std::string numStr;
            Src->getSubStr(t->Index, t->Size, numStr);
            int64_t num = strToInt(numStr);

            // <iR> + <i32> expects integer to have a maximum size of 32 bits
            // Check if the requirement is meet otherwise throw error
            if (num >> 32 != 0) {
                printTokenError(
                    "Register offset immediate does not fit into 32-bit value",
                    *t);
                return false;
            }
            regOff->Immediate.U32 = (uint32_t)num;

            // TODO: Register offset position is not correct
            regOff->Index = t->Index;
            regOff->LineRow = t->LineRow;
            regOff->LineCol = t->LineCol;
            regOff->Layout |= RO_LAYOUT_IR_INT;
            instr->Params.push_back(regOff);
            t = eatToken();
        } else {
            printTokenError(
                "Expected closing bracket after immediate offset inside "
                "register offset ]",
                *t);
            return false;
        }
    } else if (t->Type == TokenType::REGISTER_DEFINITION) {
        regOff->Offset =
            new RegisterId(t->Index, t->Size, t->LineRow, t->LineCol, t->Tag);
        t = eatToken();
        if (t->Type == TokenType::ASTERISK) {
            t = eatToken();
        } else {
            printTokenError("Expected * after offset inside register offset",
                            *t);
            return false;
        }

        std::string numStr;
        Src->getSubStr(t->Index, t->Size, numStr);
        int64_t num = strToInt(numStr);

        // <iR> +/- <iR> * <i16> expects integer to have a maximum size of 16
        // bits Check if the requirement is meet otherwise throw error
        if (num >> 16 != 0) {
            printTokenError(
                "Register offset immediate does not fit into 16-bit value", *t);
            return false;
        }
        regOff->Immediate.U16 = (uint16_t)num;
        t = eatToken();

        if (t->Type == TokenType::RIGHT_SQUARE_BRACKET) {
            // TODO: Register offset position is not correct
            regOff->Index = t->Index;
            regOff->LineRow = t->LineRow;
            regOff->LineCol = t->LineCol;
            regOff->Layout |= RO_LAYOUT_IR_IR_INT;
            instr->Params.push_back(regOff);
        } else {
            printTokenError("Expectd closing bracket after factor", *t);
            return false;
        }

    } else {
        printTokenError("Expected register or int number as offset", *t);
        return false;
    }
    return true;
}

/**
 * Builds AST for static section
 * @param sec Parent section
 * @return On valid input returns true otherwise false
 */
bool Parser::parseSectionVars(ASTSection* sec) {
    bool validSec = true;
    Token* tok = eatToken();

    // Ignore end of line
    if (tok->Type == TokenType::EOL) {
        tok = eatToken();
    }

    while (tok->Type != TokenType::RIGHT_CURLY_BRACKET) {
        Identifier* id = nullptr;
        TypeInfo* typeInfo = nullptr;
        ASTNode* val = nullptr;

        // Variable name
        if (tok->Type != TokenType::IDENTIFIER) {
            printTokenError("Expected static variable identifier", *tok);
            validSec = false;
            break;
        }

        std::string idName;
        Src->getSubStr(tok->Index, tok->Size, idName);
        id = new Identifier(tok->Index, tok->Size, tok->LineRow, tok->LineCol,
                            idName);

        // Colon
        tok = eatToken();
        if (tok->Type != TokenType::COLON) {
            printTokenError("Expected colon after variable identifier", *tok);
            validSec = false;
            break;
        }

        // Type
        tok = eatToken();
        if (tok->Type != TokenType::TYPE_INFO) {
            printTokenError("Expected type info in variable declaration", *tok);
            validSec = false;
            break;
        }
        typeInfo = new TypeInfo(tok->Index, tok->Size, tok->LineRow,
                                tok->LineCol, tok->Tag);

        // Equals
        tok = eatToken();
        if (tok->Type != TokenType::EQUALS_SIGN) {
            printTokenError(
                "Expected equals sign after type info in variable declaration",
                *tok);
            validSec = false;
            break;
        }

        tok = eatToken();
        std::string tokString;
        Src->getSubStr(tok->Index, tok->Size, tokString);

        if (tok->Type == TokenType::STRING) {
            ASTString* str = new ASTString(tok->Index, tok->Size, tok->LineRow,
                                           tok->LineCol, tokString);
            val = dynamic_cast<ASTNode*>(str);
        } else if (tok->Type == TokenType::INTEGER_NUMBER) {
            uint64_t intVal = strToInt(tokString);
            ASTInt* integer = new ASTInt(tok->Index, tok->Size, tok->LineRow,
                                         tok->LineCol, intVal);
            val = dynamic_cast<ASTNode*>(integer);
        } else if (tok->Type == TokenType::FLOAT_NUMBER) {
            double floatVal = std::stod(tokString);
            ASTFloat* fl = new ASTFloat(tok->Index, tok->Size, tok->LineRow,
                                        tok->LineCol, floatVal);
            val = dynamic_cast<ASTNode*>(fl);
        } else {
            printTokenError(
                "Expected string, float or integer as variable value", *tok);
            validSec = false;
            break;
        }

        tok = eatToken();
        if (tok->Type != TokenType::EOL) {
            printTokenError("Expected new line after variable declaration",
                            *tok);
            validSec = false;
            break;
        }

        uint32_t varSize = (val->Index + val->Size) - id->Index;
        sec->Body.push_back(new ASTVariable(id->Index, varSize, id->LineRow,
                                            id->LineCol, id, typeInfo, val));
        tok = eatToken();
    }

    return validSec;
}

/**
 * Parses the code section
 * @return On valid input returns true otherwise false
 */
bool Parser::parseSectionCode() {
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

            if (t->Type == TokenType::END_OF_FILE ||
                t->Type == TokenType::RIGHT_CURLY_BRACKET) {
                state = ParseState::END;
                continue;
            }

            switch (t->Type) {
            case TokenType::INSTRUCTION: {
                std::string instrName;
                Src->getSubStr(t->Index, t->Size, instrName);
                instr = new Instruction(t->Index, t->Size, t->LineRow,
                                        t->LineCol, instrName, t->Tag);
                FileNode->SecCode->Body.push_back(instr);

                Token* peek = peekToken();
                if (peek->Type == TokenType::END_OF_FILE) {
                    printTokenError("Unexpected end of file after instruction",
                                    *t);
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
                LabelDef* label = new LabelDef(t->Index, t->Size, t->LineRow,
                                               t->LineCol, labelName);
                FileNode->SecCode->Body.push_back(label);

                Token* peek = peekToken();
                if (peek->Type != TokenType::EOL) {
                    printTokenError("Expected new line after label definition",
                                    *t);
                    return false;
                }
                t = eatToken(); // TODO: BUG ?
            } break;
            default:
                printTokenError("Unexpected token in function body", *t);
                return false;
                break;
            }
        } break;
        case ParseState::INSTR_BODY: {
            if (t->Type == TokenType::TYPE_INFO) {
                TypeInfo* typeInfo = new TypeInfo(t->Index, t->Size, t->LineRow,
                                                  t->LineCol, t->Tag);
                instr->Params.push_back(typeInfo);
                t = eatToken();
            }

            bool endOfParamList = false;
            while (!endOfParamList) {
                switch (t->Type) {
                case TokenType::IDENTIFIER: {
                    std::string idName;
                    Src->getSubStr(t->Index, t->Size, idName);
                    Identifier* id = new Identifier(
                        t->Index, t->Size, t->LineRow, t->LineCol, idName);
                    instr->Params.push_back(id);
                } break;
                case TokenType::REGISTER_DEFINITION: {
                    RegisterId* reg = new RegisterId(
                        t->Index, t->Size, t->LineRow, t->LineCol, t->Tag);
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
                    ASTInt* iNum = new ASTInt(t->Index, t->Size, t->LineRow,
                                              t->LineCol, num);
                    instr->Params.push_back(iNum);
                } break;
                case TokenType::FLOAT_NUMBER: {
                    std::string floatStr;
                    Src->getSubStr(t->Index, t->Size, floatStr);
                    double num = std::atof(floatStr.c_str());
                    ASTFloat* iNum = new ASTFloat(t->Index, t->Size, t->LineRow,
                                                  t->LineCol, num);
                    instr->Params.push_back(iNum);
                } break;
                default:
                    printTokenError("Expected parameter", *t);
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
 * Builds the abstract syntax tree
 * @return On valid input returns true otherwise false
 */
bool Parser::buildAST() {
    bool validInput = true;
    Token* currentToken = eatToken();
    while (currentToken->Type != TokenType::END_OF_FILE) {
        // Ignore EOL
        if (currentToken->Type == TokenType::EOL) {
            currentToken = eatToken();
            continue;
        }

        // Section identifier
        if (currentToken->Type != TokenType::IDENTIFIER) {
            printTokenError("Expected section identifier in global scope",
                            *currentToken);
            validInput = false;
            break;
        }
        Token* secToken = currentToken;

        // identifer {
        currentToken = eatToken();
        if (currentToken->Type != TokenType::LEFT_CURLY_BRACKET) {
            printTokenError("Expected { after section identifier",
                            *currentToken);
            validInput = false;
            break;
        }

        std::string secName;
        Src->getSubStr(secToken->Index, secToken->Size, secName);

        // TODO: Check for section redefiniton
        if (secName == "static") {
            FileNode->SecStatic = new ASTSection(
                secToken->Index, secToken->Size, secToken->LineRow,
                secToken->LineCol, secName, ASTSectionType::STATIC);
            if (!parseSectionVars(FileNode->SecStatic)) {
                validInput = false;
                break;
            }
        } else if (secName == "global") {
            FileNode->SecGlobal = new ASTSection(
                secToken->Index, secToken->Size, secToken->LineRow,
                secToken->LineCol, secName, ASTSectionType::GLOBAL);
            if (!parseSectionVars(FileNode->SecGlobal)) {
                validInput = false;
                break;
            }
        } else if (secName == "code") {
            FileNode->SecCode = new ASTSection(
                secToken->Index, secToken->Size, secToken->LineRow,
                secToken->LineCol, secName, ASTSectionType::CODE);
            if (!parseSectionCode()) {
                validInput = false;
                break;
            }
        } else {
            printTokenError("Unknown section type", *secToken);
            validInput = false;
            break;
        }

        currentToken = eatToken();
    }

    return validInput;
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

    // Check if instr has no parameters and see if definiton accepts no
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
            printError(Src, instr->Index, instr->Name.size(), instr->LineRow,
                       instr->LineCol, "Expected parameters found none");
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
                    printError(Src, typeInfo->Index, 3, typeInfo->LineRow,
                               typeInfo->LineCol,
                               "Expected int type found float type");
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
                    printError(Src, typeInfo->Index, 3, typeInfo->LineRow,
                               typeInfo->LineCol,
                               "Expected float type found int type");
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
                // TODO: What about flag register ?
                if (regId->Id < 0x1 || regId->Id > 0x15) {
                    printError(Src, regId->Index, 3, regId->LineRow,
                               regId->LineCol, "Expected integer register");
                    break;
                }
                nextNode = &currentNode->Children[n];
            } break;
            case InstrParamType::FLOAT_REG: {
                if (astNode->Type != ASTType::REGISTER_ID) {
                    break;
                }
                RegisterId* regId = dynamic_cast<RegisterId*>(astNode);
                if (regId->Id < 0x16 || regId->Id > 0x26) {
                    printError(Src, regId->Index, 3, regId->LineRow,
                               regId->LineCol, "Expected float register");
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
                ASTInt* num = dynamic_cast<ASTInt*>(astNode);
                num->DataType = type->DataType;
                nextNode = &currentNode->Children[n];
            } break;
            case InstrParamType::FLOAT_NUM: {
                if (astNode->Type != ASTType::FLOAT_NUMBER) {
                    break;
                }

                ASTFloat* num = dynamic_cast<ASTFloat*>(astNode);
                num->DataType = type->DataType;
                nextNode = &currentNode->Children[n];
            } break;
            case InstrParamType::SYS_INT: {
                if (astNode->Type != ASTType::INTEGER_NUMBER) {
                    break;
                }

                ASTInt* num = dynamic_cast<ASTInt*>(astNode);
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
        printError(Src, instr->Index, instr->Name.size(), instr->LineRow,
                   instr->LineCol,
                   "Error no matching parameter list found for instruction");
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
 * Checks if type and values of global and static variables match
 * @return On valid input return true otherwise false
 */
bool Parser::typeCheckVars() {
    // Create vector contain both static and global vars
    std::vector<ASTNode*> vars;
    vars.insert(vars.begin(), FileNode->SecStatic->Body.begin(),
                FileNode->SecStatic->Body.end());
    vars.insert(vars.end(), FileNode->SecGlobal->Body.begin(),
                FileNode->SecGlobal->Body.end());

    // Check for variable redefinitons
    for (ASTNode* node : vars) {
        ASTVariable* var = dynamic_cast<ASTVariable*>(node);

        // Look if variable name has already been defined in the range of
        // already checked vars
        bool found = false;
        uint32_t i = 0;
        ASTVariable* refVar = dynamic_cast<ASTVariable*>(vars[i]);
        while (refVar != var && i < vars.size()) {
            if (var->Id->Name == refVar->Id->Name) {
                found = true;
                break;
            }
            i++;
            refVar = dynamic_cast<ASTVariable*>(vars[i]);
        }

        if (found) {
            printError(Src, var->Index, var->Size, var->LineRow, var->LineCol,
                       "Variable redefiniton");
            continue;
        }
    }

    return true;
}

/**
 * Performs a complete type checking pass over the AST
 * @return Returns true if no errors occured otherwise false
 */
bool Parser::typeCheck() {
    // Tracks if an error occured while type checking
    bool typeCheckError = false;
    if (!typeCheckVars()) {
        typeCheckError = true;
    }

    // Check if Global AST node has no children which means the main function is
    // missing for sure
    if (FileNode->SecCode->Body.size() == 0) {
        std::cout << "[Type Checker] Missing main label\n";
        return false;
    }

    // Try to find main entry point
    LabelDef* mainEntry = nullptr;
    for (uint32_t i = 0; i < FileNode->SecCode->Body.size(); i++) {
        ASTNode* node = FileNode->SecCode->Body[i];
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

    // Type check complete AST. This assumes that the build AST generated a
    // valid AST
    for (const auto& globElem : FileNode->SecCode->Body) {
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
                printError(Src, label->Index, label->Name.size(),
                           label->LineRow, label->LineCol,
                           "Label is already defined");
                typeCheckError = true;
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
            printError(Src, labelRef->Index, labelRef->Name.size(),
                       labelRef->LineRow, labelRef->LineCol,
                       "Unresolved label");
            typeCheckError = true;
        }
    }

    return !typeCheckError;
}
