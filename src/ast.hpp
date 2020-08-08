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

#pragma once
#include "instruction.hpp"
#include "token.hpp"
#include <cstdint>
#include <string>
#include <vector>

enum class ASTType {
    GLOBAL,
    FUNCTION_DEFINTION,
    LABEL_DEFINITION,
    IDENTIFIER,
    INSTRUCTION,
    FLOAT_NUMBER,
    INTEGER_NUMBER,
    REGISTER_ID,
    REGISTER_OFFSET,
    TYPE_INFO,
};

enum class RegisterLayout {
    IR,
    IR_INT,
    IR_IR_INT,
};

class ASTNode {
  public:
    ASTNode(ASTType type);
    ASTNode(ASTType type, uint32_t pos, uint32_t lineNr, uint32_t lineCol);
    ASTType Type;
    uint32_t Position;
    uint32_t LineNumber;
    uint32_t LineColumn;
    virtual ~ASTNode() = default;
};

class Global : public ASTNode {
  public:
    Global(uint32_t pos, uint32_t lineNr, uint32_t lineCol);
    std::vector<ASTNode*> Body;
};

class FuncDef : public ASTNode {
  public:
    FuncDef(uint32_t pos, uint32_t lineNr, uint32_t lineCol, std::string name);
    std::string Name;
    std::vector<ASTNode*> Body;
};

class LabelDef : public ASTNode {
  public:
    LabelDef(uint32_t pos, uint32_t lineNr, uint32_t lineCol, std::string name);
    std::string Name;
};

class Identifier : public ASTNode {
  public:
    Identifier(uint32_t pos,
               uint32_t lineNr,
               uint32_t lineCol,
               std::string name);
    std::string Name;
};

class Instruction : public ASTNode {
  public:
    Instruction(uint32_t pos,
                uint32_t lineNr,
                uint32_t lineCol,
                std::string name);
    std::string Name;
    std::vector<ASTNode*> Params;
    InstrParamList* ParamList = nullptr;
};

class FloatNumber : public ASTNode {
  public:
    FloatNumber(uint32_t pos, uint32_t lineNr, uint32_t lineCol, double num);
    double Num;
    uint8_t DataType;
};

class IntegerNumber : public ASTNode {
  public:
    IntegerNumber();
    IntegerNumber(uint32_t pos, uint32_t lineNr, uint32_t lineCol, int64_t num);
    int64_t Num;
    uint8_t DataType;
};

class RegisterId : public ASTNode {
  public:
    RegisterId(uint32_t pos, uint32_t lineNr, uint32_t lineCol, uint8_t id);
    uint8_t Id;
};

class RegisterOffset : public ASTNode {
  public:
    RegisterOffset();
    RegisterOffset(uint32_t pos,
                   uint32_t lineNr,
                   uint32_t lineCol,
                   RegisterLayout layout,
                   RegisterId* base,
                   RegisterId* offset,
                   IntegerNumber* immediate);
    RegisterLayout Layout;
    RegisterId* Base = nullptr;
    RegisterId* Offset = nullptr;
    IntegerNumber* Immediate = nullptr;
    bool Signed = false;
};

class TypeInfo : public ASTNode {
  public:
    TypeInfo(uint32_t pos, uint32_t lineNr, uint32_t lineCol, uint8_t dataType);
    uint8_t DataType;
};
