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

#pragma once
#include "asm/asm.hpp"
#include "token.hpp"
#include <cstdint>
#include <string>
#include <vector>

enum class ASTType {
    GLOBAL,
    LABEL_DEFINITION,
    IDENTIFIER,
    INSTRUCTION,
    FLOAT_NUMBER,
    INTEGER_NUMBER,
    REGISTER_ID,
    REGISTER_OFFSET,
    TYPE_INFO,
};

// clang-format off
// These values are used to define the layout of a register offset.
// All of these are positive to make them the negative layout version set the
// first bit.
// RO_LAYOUT_IR_INT -> 0010 1111  = <iR> + <iR> * <i16>
// NEGATIVE MASK    -> 1000 0000
// =========== OR ==============
//                  -> 1010 1111  = <iR> - <iR> * <i16>
// clang-format on
constexpr uint8_t RO_LAYOUT_IR = 0x4F;        // <iR>
constexpr uint8_t RO_LAYOUT_IR_INT = 0x2F;    // <iR> + <i32>
constexpr uint8_t RO_LAYOUT_IR_IR_INT = 0x1F; // <iR> + <iR> * <i16>

/**
 * This is used to tell the generator what the identifier referes to
 */
enum class IdentifierType {
    NONE = 0,
    FUNC_REF,
    LABEL_REF,
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
                std::string name,
                uint32_t asmDefIndex);
    std::string Name;
    std::vector<ASTNode*> Params;
    uint32_t ASMDefIndex = 0;
    uint8_t Opcode = 0;
    uint8_t EncodingFlags = 0;
    // TODO: This should not be needed by the generator
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

union ROInt {
    uint16_t U16;
    uint32_t U32 = 0;
};

class RegisterOffset : public ASTNode {
  public:
    RegisterOffset();
    RegisterOffset(uint32_t pos,
                   uint32_t lineNr,
                   uint32_t lineCol,
                   uint8_t layout,
                   RegisterId* base,
                   RegisterId* offset);
    uint8_t Layout = 0;
    RegisterId* Base = nullptr;
    RegisterId* Offset = nullptr;
    ROInt Immediate;
};

class TypeInfo : public ASTNode {
  public:
    TypeInfo(uint32_t pos, uint32_t lineNr, uint32_t lineCol, uint8_t dataType);
    uint8_t DataType;
};

/**
 * This is used by the parser to check if a label reference is resolved. In
 * the generator stage this is used to fill out the placeholder addresses of
 * label references with the VAddr member. The VAddr member will be filled
 * out by the generator once it loops through the AST and generates the
 * instructions.
 */
struct LabelDefLookup {
    /** Non owning pointer to the label definition node */
    LabelDef* Def = nullptr;
    /** Used by the generator to lookup function references */
    uint64_t VAddr = 0;
};
