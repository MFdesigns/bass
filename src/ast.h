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
#include "token.h"
#include <cstdint>
#include <string>
#include <vector>

enum class ASTType {
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
    REG = 0x1,
    REG_P_I8 = 0x2,
    REG_M_I8 = 0x3,
    REG_P_I16 = 0x4,
    REG_M_I16 = 0x5,
    REG_P_I32 = 0x6,
    REG_M_I32 = 0x7,
    REG_P_REG_T_I8 = 0x10,
    REG_M_REG_T_I8 = 0x11,
    REG_P_REG_T_I16 = 0x12,
    REG_M_REG_T_I16 = 0x13,
};

enum class UVMType {
    NONE,
    I8,
    I16,
    I32,
    I64,
    F32,
    F64,
};

UVMType getUVMTypeFromName(std::string& typeName);
uint8_t getRegisterTypeFromName(std::string& regName);

class ASTNode {
  public:
    ASTNode(ASTType type, uint32_t pos, uint32_t lineNr, uint32_t lineCol);
    ASTType Type;
    uint32_t Position;
    uint32_t LineNumber;
    uint32_t LineColumn;
    virtual ~ASTNode() = default;
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
    std::vector<ASTNode*> Parameters;
};

class FloatNumber : public ASTNode {
  public:
    FloatNumber(uint32_t pos, uint32_t lineNr, uint32_t lineCol, double num);
    double Num;
};

class IntegerNumber : public ASTNode {
  public:
    IntegerNumber(uint32_t pos,
                  uint32_t lineNr,
                  uint32_t lineCol,
                  uint64_t num);
    uint64_t Num;
};

class RegisterId : public ASTNode {
  public:
    RegisterId(uint32_t pos, uint32_t lineNr, uint32_t lineCol, uint8_t id);
    uint8_t Id;
};

class RegisterOffset : public ASTNode {
  public:
    RegisterOffset(uint32_t pos,
                   uint32_t lineNr,
                   uint32_t lineCol,
                   RegisterLayout layout,
                   RegisterId* base,
                   RegisterId* offset,
                   IntegerNumber* immediate);
    RegisterLayout Layout;
    RegisterId* Base;
    RegisterId* Offset;
    IntegerNumber* Immediate;
};

class TypeInfo : public ASTNode {
  public:
    TypeInfo(uint32_t pos, uint32_t lineNr, uint32_t lineCol, UVMType dataType);
    UVMType DataType;
};
