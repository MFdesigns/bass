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
#include "ast.hpp"
#include "fileBuffer.hpp"
#include <cstdint>
#include <filesystem>
#include <vector>

constexpr uint8_t SEC_NAME_STRINGS = 0x1;
constexpr uint8_t SEC_META_DATA = 0x2;
constexpr uint8_t SEC_DEBUG = 0x3;
constexpr uint8_t SEC_STATIC = 0x4;
constexpr uint8_t SEC_CODE = 0x5;

constexpr uint8_t SEC_PERM_READ = 0b1000'0000;
constexpr uint8_t SEC_PERM_WRITE = 0b0100'0000;
constexpr uint8_t SEC_PERM_EXECUTE = 0b0010'0000;

// These flags are used to generate the register offset layout byte
constexpr uint8_t RO_ENC_BASE_IR = 0b0100'0000;
constexpr uint8_t RO_ENC_SIGNED = 0b0010'0000;
constexpr uint8_t RO_ENC_OFFSET_IR = 0b0000'0100;
constexpr uint8_t RO_ENC_OFFSET_I8 = 0b0000'1000;
constexpr uint8_t RO_ENC_OFFSET_I16 = 0b0000'1100;
constexpr uint8_t RO_ENC_OFFSET_I32 = 0b0001'0000;
constexpr uint8_t RO_ENC_FACTOR_I8 = 0b0000'0001;
constexpr uint8_t RO_ENC_FACTOR_I16 = 0b0000'0010;

constexpr uint32_t HEADER_SIZE = 0x60;
constexpr uint32_t SEC_TABLE_ENTRY_SIZE = 0x1A;

typedef unsigned long long vAddr;

struct SecNameString {
    SecNameString(std::string str, vAddr addr);
    std::string Str;
    vAddr Addr = 0;
};

struct Section {
    uint8_t Type = 0;
    uint8_t Perms = 0;
    uint64_t StartAddr = 0;
    uint64_t Size = 0;
    SecNameString* SecName = nullptr;
};

class Generator {
  public:
    Generator(Global* ast, std::filesystem::path* p);
    ~Generator();
    void genBinary();

  private:
    Global* AST = nullptr;           // Non owning pointer
    std::filesystem::path* FilePath; // Non owning pointer
    FileBuffer* Buffer = nullptr;
    Section* SecNameTable = nullptr;
    Section* SecCode = nullptr;
    std::vector<SecNameString> SecNameStrings;
    uint64_t Cursor = 0;
    vAddr StartAddr = 0;
    void createHeader();
    void createSectionTable();
    void emitRegisterOffset(RegisterOffset* regOff, uint8_t* tempBuff);
    void emitInstruction(Instruction* instr);
    void createByteCode();
    void fillSectionTable();
    void writeFile();
};
