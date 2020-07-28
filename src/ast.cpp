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

UVMType getUVMTypeFromName(std::string& typeName) {
    UVMType type = UVMType::NONE;
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

uint8_t getRegisterTypeFromName(std::string& regName) {
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

ASTNode::ASTNode(ASTType type, uint32_t pos, uint32_t lineNr, uint32_t lineCol)
    : Type(type), Position(pos), LineNumber(lineNr), LineColumn(lineCol) {}

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
                             uint64_t num)
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
