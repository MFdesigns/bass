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
#include "token.h"
#include <array>
#include <cstdint>
#include <iostream>
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

class Assembler {
  public:
    Assembler(uint8_t* source, uint32_t sourceSize);
    bool tokenize();

  private:
    uint8_t* Source;
    uint32_t SourceSize;
    uint32_t Cursor;
    uint32_t CursorLineRow;
    uint32_t CursorLineColumn;
    std::vector<Token> Tokens;
    void tokenizerError(const char* msg);
    void incLineRow();
    bool eatChar(uint8_t& out);
    bool eatChars(uint32_t count, uint8_t& out);
    bool peekChar(uint8_t& out);
    void addToken(TokenType type,
                  uint32_t index,
                  uint32_t lineRow,
                  uint32_t lineColumn,
                  uint32_t size);
    bool static isInstruction(std::string& token);
    bool static isTypeInfo(std::string& token);
    bool static isRegister(std::string& token);
};
