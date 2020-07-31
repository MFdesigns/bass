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

#include "ast.hpp"

ASTNode::ASTNode(ASTType type, uint32_t pos, uint32_t lineNr, uint32_t lineCol)
    : Type(type), Position(pos), LineNumber(lineNr), LineColumn(lineCol) {}

Global::Global(uint32_t pos, uint32_t lineNr, uint32_t lineCol)
    : ASTNode(ASTType::GLOBAL, pos, lineNr, lineCol) {}

FuncDef::FuncDef(uint32_t pos,
                 uint32_t lineNr,
                 uint32_t lineCol,
                 std::string name)
    : ASTNode(ASTType::FUNCTION_DEFINTION, pos, lineNr, lineCol), Name(name) {}

LabelDef::LabelDef(uint32_t pos,
                   uint32_t lineNr,
                   uint32_t lineCol,
                   std::string name)
    : ASTNode(ASTType::LABEL_DEFINITION, pos, lineNr, lineCol), Name(name) {}

Identifier::Identifier(uint32_t pos,
                       uint32_t lineNr,
                       uint32_t lineCol,
                       std::string name)
    : ASTNode(ASTType::IDENTIFIER, pos, lineNr, lineCol), Name(name) {}

Instruction::Instruction(uint32_t pos,
                         uint32_t lineNr,
                         uint32_t lineCol,
                         std::string name)
    : ASTNode(ASTType::INSTRUCTION, pos, lineNr, lineCol), Name(name) {}

FloatNumber::FloatNumber(uint32_t pos,
                         uint32_t lineNr,
                         uint32_t lineCol,
                         double num)
    : ASTNode(ASTType::FLOAT_NUMBER, pos, lineNr, lineCol), Num(num) {}

IntegerNumber::IntegerNumber(uint32_t pos,
                             uint32_t lineNr,
                             uint32_t lineCol,
                             int64_t num)
    : ASTNode(ASTType::INTEGER_NUMBER, pos, lineNr, lineCol), Num(num) {}

RegisterId::RegisterId(uint32_t pos,
                       uint32_t lineNr,
                       uint32_t lineCol,
                       uint8_t id)
    : ASTNode(ASTType::REGISTER_ID, pos, lineNr, lineCol), Id(id) {}

RegisterOffset::RegisterOffset(uint32_t pos,
                               uint32_t lineNr,
                               uint32_t lineCol,
                               RegisterLayout layout,
                               RegisterId* base,
                               RegisterId* offset,
                               IntegerNumber* immediate)
    : ASTNode(ASTType::REGISTER_OFFSET, pos, lineNr, lineCol), Layout(layout),
      Base(base), Offset(offset), Immediate(immediate) {}

TypeInfo::TypeInfo(uint32_t pos,
                   uint32_t lineNr,
                   uint32_t lineCol,
                   UVMType dataType)
    : ASTNode(ASTType::TYPE_INFO, pos, lineNr, lineCol), DataType(dataType) {}
