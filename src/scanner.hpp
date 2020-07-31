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
#include "token.hpp"
#include <array>
#include <cstdint>
#include <string>
#include <vector>

const static std::array<std::string, 51> INSTRUCTIONS = {
    "nop",  "push", "pop",  "load", "loadf", "store", "storef", "copy", "copyf",
    "exit", "call", "ret",  "sys",  "lea",   "add",   "addf",   "sub",  "subf",
    "mul",  "mulf", "div",  "divf", "sqrt",  "and",   "or",     "xor",  "not",
    "lsh",  "rsh",  "srsh", "b2l",  "s2l",   "i2l",   "b2sl",   "s2sl", "i2sl",
    "f2d",  "d2f",  "i2f",  "i2d",  "f2i",   "d2i",   "cmp",    "cmpf", "jmp",
    "je",   "jne",  "jgt",  "jlt",  "jge",   "jle"};
const static std::array<std::string, 6> TYPE_INFOS = {"i8",  "i16", "i32",
                                                      "i64", "f32", "f64"};
const static std::array<std::string, 35> REGISTERS = {
    "ip",  "bp", "sp",  "r0",  "r1",  "r2",  "r3",  "r4",  "r5",
    "r6",  "r7", "r8",  "r9",  "r10", "r11", "r12", "r13", "r14",
    "r15", "f0", "f1",  "f2",  "f3",  "f4",  "f5",  "f6",  "f7",
    "f8",  "f9", "f10", "f11", "f12", "f13", "f14", "f15",
};

class Source {
  public:
    Source(uint8_t* data, uint32_t size);
    ~Source();
    uint32_t getSize();
    uint8_t* getData();
    bool getChar(uint32_t index, uint8_t& c);
    bool getSubStr(uint32_t index, uint32_t size, std::string& out);
    bool getLine(uint32_t index, std::string& out, uint32_t& lineIndex);

  private:
    uint8_t* Data = nullptr; // TODO: Make const
    const uint32_t Size = 0;
};

class Scanner {
  public:
    Scanner(Source* src, std::vector<Token>* outTokens);
    bool scanSource();

  private:
    uint32_t Cursor = 0;
    uint32_t CursorLineRow = 1;
    uint32_t CursorLineColumn = 1;
    Source* Src = nullptr;
    std::vector<Token>* Tokens = nullptr;
    void throwError(const char* msg, uint32_t start);
    bool scanWord(uint32_t& outSize);
    bool isRegister(std::string& token);
    bool isTypeInfo(std::string& token);
    bool isInstruction(std::string& token);
    void addToken(TokenType type,
                  uint32_t index,
                  uint32_t lineRow,
                  uint32_t lineColumn,
                  uint32_t size);
    bool peekChar(uint8_t& out);
    bool eatChars(uint32_t count, uint8_t& out);
    bool eatChar(uint8_t& out);
    void incLineRow();
    void incCursor();
};
