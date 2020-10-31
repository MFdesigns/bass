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

#include "generator.hpp"
#include <array>
#include <cstring>
#include <fstream>

SecNameString::SecNameString(std::string str, vAddr addr)
    : Str(str), Addr(addr) {}

/**
 * Constructs a new Generator
 * @param ast Pointer to AST
 * @param p Pointer to output file path
 * @param funcDefs Pointer to array of FuncDefLookup
 */
Generator::Generator(Global* ast,
                     std::filesystem::path* p,
                     std::vector<FuncDefLookup>* funcDefs)
    : AST(ast), FilePath(p), FuncDefs(funcDefs) {
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
    constexpr uint32_t secTableSize =
        2 * SEC_TABLE_ENTRY_SIZE +
        4; // + 4 because of the section table size uint32_t at the beginning
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
    std::ofstream stream{*FilePath, std::ios::binary};
    Buffer->writeToStream(stream);
    stream.close();
}

void Generator::emitRegisterOffset(RegisterOffset* regOff, uint8_t* out) {
    // Encode RO layout byte
    out[0] = regOff->Layout;

    // Encode base register
    out[1] = regOff->Base->Id;

    if ((regOff->Layout & RO_LAYOUT_IR_INT) == RO_LAYOUT_IR_INT) {
        std::memcpy(&out[2], &regOff->Immediate.U16, 4);
    } else if ((regOff->Layout & RO_LAYOUT_IR_IR_INT) == RO_LAYOUT_IR_IR_INT) {
        out[2] = regOff->Offset->Id;
        std::memcpy(&out[3], &regOff->Immediate.U16, 2);
    }
}

/**
 * Adds a function reference which has to be resolved
 * @param funcRef Non owning pointer to function reference
 * @param vAddr Address to placeholder in output file
 */
void Generator::addResolvableFuncRef(Identifier* funcRef, uint64_t vAddr) {
    // Find function definiton which this reference referes to
    FuncDefLookup* funcDef = nullptr;
    for (uint32_t i = 0; i < FuncDefs->size(); i++) {
        FuncDefLookup* lookup = &(*FuncDefs)[i];
        if (lookup->Def->Name == funcRef->Name) {
            funcDef = lookup;
            break;
        }
    }

    // TODO: What if funcDef is not found?
    ResFuncRefs.push_back(ResolvableFuncRef{vAddr, funcDef});
}

/**
 * Emits a given instruction
 * @param instr Instruction to emit
 */
void Generator::emitInstruction(Instruction* instr) {
    constexpr uint32_t MAX_INSTR_SIZE = 15;
    // Temporary instruction mem
    uint8_t temp[MAX_INSTR_SIZE] = {0};
    uint32_t instrSize = 0;

    // Set opcode
    temp[0] = instr->Opcode;
    instrSize++;

    // Emits parameters
    for (auto& param : instr->Params) {
        switch (param->Type) {
        case ASTType::IDENTIFIER: {
            Identifier* id = dynamic_cast<Identifier*>(param);
            if (id->IdType == IdentifierType::FUNC_REF) {
                addResolvableFuncRef(id, Cursor + instrSize);
            } else if (id->IdType == IdentifierType::LABEL_REF) {
                // TODO: Add Resolvable label reference
            }
            instrSize += 8;
        } break;
        case ASTType::FLOAT_NUMBER: {
            FloatNumber* num = dynamic_cast<FloatNumber*>(param);
            if (num->DataType == UVM_TYPE_F32) {
                float typedNum = (float)num->Num;
                std::memcpy(&temp[instrSize], &typedNum, 4);
                instrSize += 4;
            } else if (num->DataType == UVM_TYPE_F64) {
                std::memcpy(&temp[instrSize], &num->Num, 8);
                instrSize += 8;
            }
        } break;
        case ASTType::INTEGER_NUMBER: {
            IntegerNumber* num = dynamic_cast<IntegerNumber*>(param);
            if (num->DataType == UVM_TYPE_I8) {
                uint8_t typedNum = (uint8_t)num->Num;
                temp[instrSize] = typedNum;
                instrSize++;
            } else if (num->DataType == UVM_TYPE_I16) {
                uint16_t typedNum = (uint16_t)num->Num;
                std::memcpy(&temp[instrSize], &typedNum, 2);
                instrSize += 2;
            } else if (num->DataType == UVM_TYPE_I32) {
                uint32_t typedNum = (uint32_t)num->Num;
                std::memcpy(&temp[instrSize], &typedNum, 4);
                instrSize += 4;
            } else if (num->DataType == UVM_TYPE_I64) {
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
            RegisterOffset* regOff = dynamic_cast<RegisterOffset*>(param);
            emitRegisterOffset(regOff, &temp[instrSize]);
            instrSize += 6;
        } break;
        case ASTType::TYPE_INFO: {
            TypeInfo* typeInfo = dynamic_cast<TypeInfo*>(param);
            if (instr->ParamList->Flags & INSTR_FLAG_ENCODE_TYPE) {
                temp[instrSize] = typeInfo->DataType;
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

/**
 * Generates bytecode and appends it to file buffer
 */
void Generator::createByteCode() {
    // Set the current Cursor as start of code section
    SecCode->StartAddr = Cursor;
    for (auto& globElem : AST->Body) {
        FuncDef* func = dynamic_cast<FuncDef*>(globElem);

        // Find function definiton in lookup table
        FuncDefLookup* lookup = nullptr;
        for (uint32_t i = 0; i < FuncDefs->size(); i++) {
            if ((*FuncDefs)[i].Def->Name == func->Name) {
                lookup = &(*FuncDefs)[i];
                break;
            }
        }

        // Add function definition address to lookup table. This will be used to
        // fill in the placeholders addresses of function calls. This assumes
        // that the function def exists in the lookup table
        lookup->VAddr = Cursor;

        // If current function is the main function set start address to this
        if (func->Name == "main") {
            StartAddr = Cursor;
        }

        // Generate function body
        for (auto& funcElem : func->Body) {
            if (funcElem->Type == ASTType::INSTRUCTION) {
                Instruction* instr = dynamic_cast<Instruction*>(funcElem);
                emitInstruction(instr);
            } else if (funcElem->Type == ASTType::LABEL_DEFINITION) {
                // TODO: ...
            }
        }
    }

    // Set code section size
    SecCode->Size = Cursor - SecCode->StartAddr;
}

/**
 * Resolves all function and label references and fills in the placeholder
 * addresses
 */
void Generator::resolveReferences() {
    for (const ResolvableFuncRef& res : ResFuncRefs) {
        uint64_t funcVAddr = res.FuncDef->VAddr;
        Buffer->write(res.VAddr, &funcVAddr, sizeof(funcVAddr));
    }

    // TODO: Resolve label refs
}

void Generator::fillSectionTable() {
    std::array<Section*, 2> tmpSections = {SecNameTable, SecCode};
    uint8_t tmpCursor = HEADER_SIZE;

    // Put section table size
    uint32_t secTableSize = tmpSections.size() * SEC_TABLE_ENTRY_SIZE;
    Buffer->write(tmpCursor, (uint8_t*)&secTableSize, 4);
    tmpCursor += 4;

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

/**
 * Generates the output UX file and writes it to disk
 */
void Generator::genBinary() {
    createHeader();
    createSectionTable();
    createByteCode();
    resolveReferences();

    Buffer->write(0x8, (uint8_t*)&StartAddr, 8);
    fillSectionTable();

    writeFile();
}
