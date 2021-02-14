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

#include "ast.hpp"

ASTNode::ASTNode(ASTType type) : Type(type) {}

ASTNode::ASTNode(ASTType type,
                 uint32_t pos,
                 uint32_t size,
                 uint32_t lineNr,
                 uint32_t lineCol)
    : Type(type), Index(pos), Size(size), LineRow(lineNr), LineCol(lineCol) {}

ASTSection::ASTSection(uint32_t pos,
                       uint32_t size,
                       uint32_t lineNr,
                       uint32_t lineCol,
                       std::string name,
                       ASTSectionType secType)
    : ASTNode(ASTType::VARIABLE, pos, size, lineNr, lineCol), Name(name),
      SecType(secType) {}

ASTVariable::ASTVariable(uint32_t pos,
                         uint32_t size,
                         uint32_t lineNr,
                         uint32_t lineCol,
                         Identifier* id,
                         TypeInfo* dataType,
                         ASTNode* val)
    : ASTNode(ASTType::SECTION, pos, size, lineNr, lineCol), Id(id),
      DataType(dataType), Val(val) {}

LabelDef::LabelDef(uint32_t pos,
                   uint32_t size,
                   uint32_t lineNr,
                   uint32_t lineCol,
                   std::string name)
    : ASTNode(ASTType::LABEL_DEFINITION, pos, size, lineNr, lineCol),
      Name(name) {}

Identifier::Identifier(uint32_t pos,
                       uint32_t size,
                       uint32_t lineNr,
                       uint32_t lineCol,
                       std::string name)
    : ASTNode(ASTType::IDENTIFIER, pos, size, lineNr, lineCol), Name(name) {}

Instruction::Instruction(uint32_t pos,
                         uint32_t size,
                         uint32_t lineNr,
                         uint32_t lineCol,
                         std::string name,
                         uint32_t asmDefIndex)
    : ASTNode(ASTType::INSTRUCTION, pos, size, lineNr, lineCol), Name(name),
      ASMDefIndex(asmDefIndex) {}

ASTFloat::ASTFloat(
    uint32_t pos, uint32_t size, uint32_t lineNr, uint32_t lineCol, double num)
    : ASTNode(ASTType::FLOAT_NUMBER, pos, size, lineNr, lineCol), Num(num) {}

ASTInt::ASTInt() : ASTNode(ASTType::INTEGER_NUMBER) {}

ASTInt::ASTInt(uint32_t pos,
               uint32_t size,
               uint32_t lineNr,
               uint32_t lineCol,
               int64_t num,
               bool isSigned)
    : ASTNode(ASTType::INTEGER_NUMBER, pos, size, lineNr, lineCol), Num(num),
      IsSigned(isSigned) {}

RegisterId::RegisterId(
    uint32_t pos, uint32_t size, uint32_t lineNr, uint32_t lineCol, uint8_t id)
    : ASTNode(ASTType::REGISTER_ID, pos, size, lineNr, lineCol), Id(id) {}

RegisterOffset::RegisterOffset() : ASTNode(ASTType::REGISTER_OFFSET){};

RegisterOffset::RegisterOffset(uint32_t pos,
                               uint32_t size,
                               uint32_t lineNr,
                               uint32_t lineCol,
                               uint8_t layout,
                               RegisterId* base,
                               RegisterId* offset)
    : ASTNode(ASTType::REGISTER_OFFSET, pos, size, lineNr, lineCol),
      Layout(layout), Base(base), Offset(offset) {}

TypeInfo::TypeInfo(uint32_t pos,
                   uint32_t size,
                   uint32_t lineNr,
                   uint32_t lineCol,
                   uint8_t dataType)
    : ASTNode(ASTType::TYPE_INFO, pos, size, lineNr, lineCol),
      DataType(dataType) {}

ASTString::ASTString(uint32_t pos,
                     uint32_t size,
                     uint32_t lineNr,
                     uint32_t lineCol,
                     std::string val)
    : ASTNode(ASTType::STRING, pos, size, lineNr, lineCol), Val(val) {}
