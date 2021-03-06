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
#include "ast.hpp"
#include "fileBuffer.hpp"
#include "parser.hpp"
#include <cstdint>
#include <filesystem>
#include <vector>

constexpr uint8_t SEC_NAME_STRINGS = 0x1;
constexpr uint8_t SEC_META_DATA = 0x2;
constexpr uint8_t SEC_DEBUG = 0x3;
constexpr uint8_t SEC_STATIC = 0x4;
constexpr uint8_t SEC_GLOBAL = 0x5;
constexpr uint8_t SEC_CODE = 0x6;

constexpr uint32_t HEADER_SIZE = 0x60;
constexpr uint32_t SEC_TABLE_ENTRY_SIZE = 0x16;

// TODO: Remove?
typedef unsigned long long vAddr;

/**
 * This is used to track which label definitons have been refered to by
 * bytecode. When the bytecode is beeing generated label pointers will be
 * filled with placeholders (null pointers) and are resolved once the location
 * of every label is known by the generator.
 */
struct ResolvableLabelRef {
    /** Virtual address to placeholder address */
    uint64_t VAddr = 0;
    /** Non owning pointer to LabelDefLookup which this LabelDef resolves to */
    LabelDefLookup* LabelDef = nullptr;
};

struct SecNameString {
    SecNameString(std::string str, vAddr addr);
    std::string Str;
    vAddr Addr = 0;
};

struct GenSection {
    uint8_t Type = 0;
    uint8_t Perms = 0;
    uint64_t StartAddr = 0;
    uint32_t Size = 0;
    uint32_t SecNameIndex = 0;
    ASTSection* SecPtr = nullptr;
};

class Generator {
  public:
    Generator(ASTFileNode* ast,
              std::filesystem::path* p,
              std::vector<LabelDefLookup>* funcDefs,
              std::vector<VarDeclaration>* varDecls);
    void genBinary();

  private:
    /** Non owning pointer to Global */
    ASTFileNode* AST = nullptr;
    /** Non owning pointer to source file */
    std::filesystem::path* FilePath = nullptr;
    /** Non owning pointer to function defintions created by parser stage */
    std::vector<LabelDefLookup>* LabelDefs = nullptr;
    /** Vector of label references which have to be resolved */
    std::vector<ResolvableLabelRef> ResLabelRefs;
    /** Non owning pointer */
    std::vector<VarDeclaration>* VarDecls;
    OutputFileBuffer Buffer;
    std::vector<GenSection> Sections;
    std::vector<SecNameString> SecNameStrings;
    uint64_t Cursor = 0;
    vAddr StartAddr = 0;
    void createHeader();
    void createSectionTable();
    void addResolvableFuncRef(Identifier* funcRef, uint64_t vAddr);
    void emitRegisterOffset(RegisterOffset* regOff, uint8_t* tempBuff);
    void emitInstruction(Instruction* instr);
    void createByteCode();
    void resolveLabelRefs();
    void fillSectionTable();
    void writeFile();
    void encodeSectionVars(GenSection& sec);
    void resolveVariableOffset(RegisterOffset* ro);
};
