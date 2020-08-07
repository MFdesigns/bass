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

#include "generator.hpp"
#include <fstream>

SecNameString::SecNameString(std::string str, vAddr addr)
    : Str(str), Addr(addr) {}

Generator::Generator(Global* ast, std::filesystem::path* p)
    : AST(ast), FilePath(p) {
    Buffer = new FileBuffer();
    // Setup section name strings
    SecNameStrings.reserve(2);
    SecNameStrings.emplace_back("Section Names", 0);
    SecNameStrings.emplace_back("Code", 0);
    // Add sections
    SecNameTable = new Section{};
    SecNameTable->Type = SEC_NAME_STRINGS;
    SecNameTable->SecName = &SecNameStrings[0];
    SecCode = new Section{};
    SecCode->Type = SEC_CODE;
    SecCode->Perms = SEC_PERM_READ | SEC_PERM_EXECUTE;
    SecCode->SecName = &SecNameStrings[1];
}

Generator::~Generator() {
    delete Buffer;
    delete SecNameTable;
    delete SecCode;
}

void Generator::createHeader() {
    // Allocate header
    Buffer->increase(HEADER_SIZE);
    // Magic
    uint8_t version = 0x1;
    uint8_t mode = 0x1;
    uint8_t data[] = {'S', 'I', 'P', 'P', version, mode};
    Buffer->write(Cursor, data, sizeof(data));
    Cursor += HEADER_SIZE;
}

void Generator::createSectionTable() {
    constexpr uint32_t secTableSize = 2 * SEC_TABLE_ENTRY_SIZE;
    uint64_t secNameVAddr = HEADER_SIZE + secTableSize;
    Cursor += secTableSize;
    // Allocate section table
    Buffer->increase(secTableSize);

    // Encode section name string entries
    uint64_t secNameSize = 0;
    SecNameTable->StartAddr = Cursor;
    for (auto& entry : SecNameStrings) {
        // Add a reference to the string entry to later be used as a pointer
        // inside the section table
        entry.Addr = Cursor;

        uint32_t strSize = entry.Str.size();
        Buffer->push(strSize);
        // Allocate the string
        uint8_t* cStr = (uint8_t*)entry.Str.c_str();
        Buffer->increase(strSize);
        Buffer->write(Cursor + 1, cStr, strSize);
        Cursor += strSize + 1;
        secNameSize += strSize + 1;
    }
    SecNameTable->Size = secNameSize;
}

void Generator::writeFile() {
    FilePath->replace_filename("main.ux");
    std::ofstream stream{*FilePath};
    Buffer->writeToStream(stream);
    stream.close();
}

void Generator::emitInstruction(Instruction* instr) {
    constexpr uint32_t MAX_INSTR_SIZE = 15;
    // Temporary instruction mem
    uint8_t temp[MAX_INSTR_SIZE] = {0};
    uint32_t instrSize = 0;
    // Get opcode
    uint8_t op = instr->ParamList->Opcode;
    if (instr->ParamList->Flags & INSTR_FLAG_TYPE_VARIANTS) {
        // Get type
        // TODO: This assumes that the first param is a type info but this
        // should be a specific member of the Instruction class
        TypeInfo* typeInfo = dynamic_cast<TypeInfo*>(instr->Params[0]);
        // Find coresponding type variant
        bool found = false;
        TypeVariant* variant = nullptr;
        uint32_t i = 0;
        while (!found && i < instr->ParamList->OpcodeVariants.size()) {
            variant = &instr->ParamList->OpcodeVariants[i];
            if (variant->Type == typeInfo->DataType) {
                found = true;
            }
            i++;
        }
        op = variant->Opcode;
    }
    temp[0] = op;
    instrSize++;

    // Emits paramters
    uint8_t type = 0;
    for (auto& param : instr->Params) {
        switch (param->Type) {
        // TODO: Generator does not know if the id is func or label refs without
        // looking at paramlist
        case ASTType::IDENTIFIER:
            instrSize += 8;
            break;
        case ASTType::FLOAT_NUMBER: {
            FloatNumber* num = dynamic_cast<FloatNumber*>(param);
            if (type == UVM_TYPE_F32) {
                float typedNum = (float)num->Num;
                std::memcpy(&temp[instrSize], &typedNum, 4);
                instrSize += 4;
            } else if (type == UVM_TYPE_F64) {
                std::memcpy(&temp[instrSize], &num->Num, 8);
                instrSize += 8;
            }
        } break;
        case ASTType::INTEGER_NUMBER: {
            IntegerNumber* num = dynamic_cast<IntegerNumber*>(param);
            if (type == UVM_TYPE_I8) {
                uint8_t typedNum = (uint8_t)num->Num;
                temp[instrSize] == typedNum;
                instrSize++;
            } else if (type == UVM_TYPE_I16) {
                uint16_t typedNum = (uint16_t)num->Num;
                std::memcpy(&temp[instrSize], &typedNum, 2);
                instrSize += 2;
            } else if (type == UVM_TYPE_I32) {
                uint32_t typedNum = (uint32_t)num->Num;
                std::memcpy(&temp[instrSize], &typedNum, 4);
                instrSize += 4;
            } else if (type == UVM_TYPE_I64) {
                std::memcpy(&temp[instrSize], &num->Num, 8);
                instrSize += 8;
            }
        } break;
        case ASTType::REGISTER_ID: {
            RegisterId* reg = dynamic_cast<RegisterId*>(param);
            temp[instrSize] = reg->Id;
            instrSize++;
        } break;
        case ASTType::REGISTER_OFFSET: {
            instrSize += 6;
        } break;
        case ASTType::TYPE_INFO: {
            TypeInfo* typeInfo = dynamic_cast<TypeInfo*>(param);
            type = typeInfo->DataType;
            if (instr->ParamList->Flags & INSTR_FLAG_ENCODE_TYPE) {
                temp[instrSize] = type;
                instrSize++;
            }
        } break;
        }
    }

    // Copy temp instr bytecode to file buffer
    Buffer->increase(instrSize);
    Buffer->write(Cursor, temp, instrSize);
    Cursor += instrSize;
}

void Generator::createByteCode() {
    SecCode->StartAddr = Cursor;
    for (auto& globElem : AST->Body) {
        FuncDef* func = dynamic_cast<FuncDef*>(globElem);
        if (func->Name == "main") {
            StartAddr = Cursor;
        }

        for (auto& funcElem : func->Body) {
            if (funcElem->Type == ASTType::INSTRUCTION) {
                Instruction* instr = dynamic_cast<Instruction*>(funcElem);
                emitInstruction(instr);
            } else if (funcElem->Type == ASTType::LABEL_DEFINITION) {
                // TODO: ...
            }
        }
    }
    SecCode->Size = Cursor - SecCode->StartAddr;
}

void Generator::fillSectionTable() {
    std::array<Section*, 2> tmpSections = {SecNameTable, SecCode};
    uint8_t tmpCursor = HEADER_SIZE;
    for (auto& sec : tmpSections) {
        uint8_t tmp[SEC_TABLE_ENTRY_SIZE] = {};
        tmp[0] = sec->Type;
        tmp[1] = sec->Perms;
        std::memcpy(&tmp[2], &sec->StartAddr, 8);
        std::memcpy(&tmp[0xA], &sec->Size, 8);
        std::memcpy(&tmp[0x12], &sec->SecName->Addr, 8);
        Buffer->write(tmpCursor, tmp, SEC_TABLE_ENTRY_SIZE);
        tmpCursor += SEC_TABLE_ENTRY_SIZE;
    }
}

void Generator::genBinary() {
    createHeader();
    createSectionTable();
    createByteCode();

    Buffer->write(0x8, (uint8_t*)&StartAddr, 8);
    fillSectionTable();

    writeFile();
}
