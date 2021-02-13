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
Generator::Generator(ASTFileNode* ast,
                     std::filesystem::path* p,
                     std::vector<LabelDefLookup>* funcDefs,
                     std::vector<VarDeclaration>* varDecls)
    : AST(ast), FilePath(p), LabelDefs(funcDefs), VarDecls(varDecls) { }

void Generator::createHeader() {
    // Allocate header
    Buffer.reserve(HEADER_SIZE);
    // Magic
    uint8_t version = 0x1;
    uint8_t mode = 0x1;
    uint8_t data[] = {'S', 'I', 'P', 'P', version, mode};
    Buffer.write(Cursor, data, sizeof(data));
    Cursor += HEADER_SIZE;
}

void Generator::createSectionTable() {
    // Figure out how many sections are actually defined and not empty. Start at count 1 because section of section name strings is alawys there
    uint32_t secTableCount = 1;
    std::vector<ASTSection*> sections = {AST->SecStatic, AST->SecGlobal, AST->SecCode};

    // Add section name strings section
    GenSection secNameStrs{};
    secNameStrs.Type = SEC_NAME_STRINGS;
    secNameStrs.SecNameIndex = SecNameStrings.size();
    SecNameStrings.emplace_back("Section Name Strings", 0);

    uint32_t secNameStrsIndex = Sections.size();
    Sections.push_back(secNameStrs);

    for (const ASTSection* sec: sections) {
        if (sec != nullptr) {
            if (sec->Body.size() > 0) {
                GenSection secEntry{};
                char* secName = nullptr;
                uint8_t secPerms = 0;

                switch (sec->SecType) {
                    case ASTSectionType::STATIC:
                    secEntry.Type = SEC_STATIC;
                        secName = "Static";
                    secPerms = SEC_PERM_READ;
                    break;
                    case ASTSectionType::GLOBAL:
                        secEntry.Type = SEC_GLOBAL;
                        secName = "Global";
                        secPerms = SEC_PERM_READ & SEC_PERM_WRITE;
                    break;
                    case ASTSectionType::CODE:
                        secEntry.Type = SEC_CODE;
                        secName = "Code";
                        secPerms = SEC_PERM_READ & SEC_PERM_EXECUTE;
                    break;
                }

                secEntry.SecNameIndex = SecNameStrings.size();
                secEntry.SecPtr = const_cast<ASTSection*>(sec);
                secEntry.Perms = secPerms;
                SecNameStrings.emplace_back(secName, 0);
                Sections.push_back(secEntry);
                secTableCount++;
            }
        }
    }

    // + 4 because of the section table size uint32_t at the beginning
    uint32_t secTableSize = secTableCount * SEC_TABLE_ENTRY_SIZE + 4;
    uint64_t secNameVAddr = HEADER_SIZE + secTableSize;
    Cursor += secTableSize;
    // Allocate section table
    Buffer.reserve(secTableSize);

    // Encode section name string entries
    uint32_t secNameSize = 0;
    Sections[secNameStrsIndex].StartAddr = Cursor;
    for (auto& entry : SecNameStrings) {
        // Add a reference to the string entry to later be used as a pointer
        // inside the section table
        entry.Addr = Cursor;

        uint8_t strSize = entry.Str.size();
        Buffer.push(&strSize, 1);
        // Allocate the string
        uint8_t* cStr = (uint8_t*)entry.Str.c_str();
        Buffer.reserve(strSize);
        Buffer.write(Cursor + 1, cStr, strSize);
        Cursor += strSize + 1;
        secNameSize += strSize + 1;
    }
    Sections[secNameStrsIndex].Size = secNameSize;
}

void Generator::writeFile() {
    FilePath->replace_filename("main.ux");
    std::ofstream stream{*FilePath, std::ios::binary};
    Buffer.writeToStream(stream);
    stream.close();
}

void Generator::emitRegisterOffset(RegisterOffset* regOff, uint8_t* out) {
    // Check if register offset is variable offset if not resolve it before
    // encoding
    if (regOff->Var != nullptr) {
        resolveVariableOffset(regOff);
    }

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
 * Adds a label reference which has to be resolved
 * @param funcRef Non owning pointer to function reference
 * @param vAddr Address to placeholder in output file
 */
void Generator::addResolvableFuncRef(Identifier* labelRef, uint64_t vAddr) {
    // Find function definiton which this reference referes to
    LabelDefLookup* labelDef = nullptr;
    for (uint32_t i = 0; i < LabelDefs->size(); i++) {
        LabelDefLookup* lookup = &(*LabelDefs)[i];
        if (lookup->Def->Name == labelRef->Name) {
            labelDef = lookup;
            break;
        }
    }

    // TODO: What if labelDef is not found?
    ResLabelRefs.push_back(ResolvableLabelRef{vAddr, labelDef});
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
            addResolvableFuncRef(id, Cursor + instrSize);
            instrSize += 8;
        } break;
        case ASTType::FLOAT_NUMBER: {
            ASTFloat* num = dynamic_cast<ASTFloat*>(param);
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
            ASTInt* num = dynamic_cast<ASTInt*>(param);
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
            if (instr->EncodingFlags & INSTR_FLAG_ENCODE_TYPE) {
                temp[instrSize] = typeInfo->DataType;
                instrSize++;
            }
        } break;
        }
    }

    // Copy temp instr bytecode to file buffer
    Buffer.reserve(instrSize);
    Buffer.write(Cursor, temp, instrSize);
    Cursor += instrSize;
}

/**
 * Generates bytecode and appends it to file buffer
 */
void Generator::createByteCode() {
    // Find code section
    GenSection* codeSec = nullptr;
    for (GenSection& sec : Sections) {
        if (sec.Type == SEC_CODE) {
            codeSec = &sec;
        }
    }

    // Set the current Cursor as start of code section
    codeSec->StartAddr = Cursor;
    for (auto& globElem : AST->SecCode->Body) {
        switch (globElem->Type) {
        case ASTType::LABEL_DEFINITION: {
            LabelDef* label = dynamic_cast<LabelDef*>(globElem);

            // Find label definiton in lookup table
            LabelDefLookup* lookup = nullptr;
            for (uint32_t i = 0; i < LabelDefs->size(); i++) {
                if ((*LabelDefs)[i].Def->Name == label->Name) {
                    lookup = &(*LabelDefs)[i];
                    break;
                }
            }

            // Add label definition address to lookup table. This will be used
            // to fill in the placeholders addresses of label calls. This
            // assumes that the function def exists in the lookup table
            lookup->VAddr = Cursor;

            // If current label is the main label set start address to this
            if (label->Name == "main") {
                StartAddr = Cursor;
            }
        } break;
        case ASTType::INSTRUCTION: {
            Instruction* instr = dynamic_cast<Instruction*>(globElem);
            emitInstruction(instr);
        }
        }
    }

    // Set code section size
    codeSec->Size = Cursor - codeSec->StartAddr;
}

/**
 * Resolves all label references and fills in the placeholder addresses
 */
void Generator::resolveLabelRefs() {
    for (const ResolvableLabelRef& res : ResLabelRefs) {
        uint64_t labelVAddr = res.LabelDef->VAddr;
        Buffer.write(res.VAddr, &labelVAddr, sizeof(labelVAddr));
    }
}

void Generator::fillSectionTable() {
    uint8_t tmpCursor = HEADER_SIZE;

    // Put section table size
    uint32_t secTableSize = Sections.size() * SEC_TABLE_ENTRY_SIZE;
    Buffer.write(tmpCursor, (uint8_t*)&secTableSize, 4);
    tmpCursor += 4;

    for (auto& sec : Sections) {
        uint8_t tmp[SEC_TABLE_ENTRY_SIZE] = {};
        tmp[0] = sec.Type;
        tmp[1] = sec.Perms;
        std::memcpy(&tmp[2], &sec.StartAddr, 8);
        std::memcpy(&tmp[0xA], &sec.Size, 4);
        std::memcpy(&tmp[0xE], &SecNameStrings[sec.SecNameIndex].Addr, 8);
        Buffer.write(tmpCursor, tmp, SEC_TABLE_ENTRY_SIZE);
        tmpCursor += SEC_TABLE_ENTRY_SIZE;
    }
}

/**
 * Encodes variables declared in a section and keeps track of the location where
 * they are encoded at
 * @param srcSec A pointer to the ASTSection containing the ASTVariables
 * @param owningSec Section table entry which will be filled out
 */
void Generator::encodeSectionVars(GenSection& sec) {
    uint64_t secStartAddr = Cursor;

    for (ASTNode* node : sec.SecPtr->Body) {
        ASTVariable* var = dynamic_cast<ASTVariable*>(node);
        uint8_t varType = var->DataType->DataType;
        uint32_t varSize = 0;

        switch (varType) {
        case UVM_TYPE_I8: {
            ASTInt* astInt = dynamic_cast<ASTInt*>(var->Val);
            Buffer.push((uint8_t*)&astInt->Num, 1);
            varSize = 1;
        } break;
        case UVM_TYPE_I16: {
            ASTInt* astInt = dynamic_cast<ASTInt*>(var->Val);
            Buffer.push((uint8_t*)&astInt->Num, 2);
            varSize = 2;
        } break;
        case UVM_TYPE_I32: {
            ASTInt* astInt = dynamic_cast<ASTInt*>(var->Val);
            Buffer.push((uint8_t*)&astInt->Num, 4);
            varSize = 4;
        } break;
        case UVM_TYPE_I64: {
            ASTInt* astInt = dynamic_cast<ASTInt*>(var->Val);
            Buffer.push((uint8_t*)&astInt->Num, 8);
            varSize = 8;
        } break;
        case UVM_TYPE_F32: {
            ASTFloat* astFloat = dynamic_cast<ASTFloat*>(var->Val);
            float val = static_cast<float>(astFloat->Num);
            Buffer.push((uint8_t*)&val, 4);
            varSize = 4;
        } break;
        case UVM_TYPE_F64: {
            ASTFloat* astFloat = dynamic_cast<ASTFloat*>(var->Val);
            Buffer.push((uint8_t*)&astFloat->Num, 8);
            varSize = 8;
        } break;
        case BASS_TYPE_STRING: {
            ASTString* str = dynamic_cast<ASTString*>(var->Val);
            uint32_t strSize = str->Val.size();
            Buffer.push(str->Val.data(), strSize);
            varSize = strSize;
        } break;
        }

        (*VarDecls)[var->VarDeclIndex].VAddr = Cursor;
        Cursor += varSize;
    }

    sec.Size = Cursor - secStartAddr;
    sec.StartAddr = secStartAddr;
}

/**
 * Fills out variable offset with register offset
 */
void Generator::resolveVariableOffset(RegisterOffset* ro) {
    constexpr uint8_t REG_IP = 0x1;

    ro->Layout = RO_LAYOUT_IR_INT | RO_LAYOUT_NEGATIVE;
    ro->Base = new RegisterId(0, 0, 0, 0, REG_IP);
    // Find variable definiton
    for (auto& varDecl : *VarDecls) {
        if (varDecl.Id->Name == ro->Var->Name) {
            ro->Immediate.U32 = static_cast<uint32_t>(Cursor - varDecl.VAddr);
            break;
        }
    }
}

/**
 * Generates the output UX file and writes it to disk
 */
void Generator::genBinary() {
    createHeader();
    createSectionTable();

    // Encode static and global sections
    for (GenSection& genSec : Sections) {
        if (genSec.Type == SEC_STATIC || genSec.Type == SEC_GLOBAL) {
            encodeSectionVars(genSec);
        }
    }

    createByteCode();
    resolveLabelRefs();

    Buffer.write(0x8, (uint8_t*)&StartAddr, 8);
    fillSectionTable();

    writeFile();
}
