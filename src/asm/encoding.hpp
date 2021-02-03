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
#include "asm.hpp"
#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

/*
    THIS FILE IS GENERATED BY THE SCRIPT 'encodingData.js' DO NOT MODIFY!
*/

const std::map<std::string, uint8_t> ASM_REGISTERS {
    {"ip", 0x1},
    {"sp", 0x2},
    {"bp", 0x3},
    {"r0", 0x5},
    {"r1", 0x6},
    {"r2", 0x7},
    {"r3", 0x8},
    {"r4", 0x9},
    {"r5", 0xA},
    {"r6", 0xB},
    {"r7", 0xC},
    {"r8", 0xD},
    {"r9", 0xE},
    {"r10", 0xF},
    {"r11", 0x10},
    {"r12", 0x11},
    {"r13", 0x12},
    {"r14", 0x13},
    {"r15", 0x14},
    {"f0", 0x16},
    {"f1", 0x17},
    {"f2", 0x18},
    {"f3", 0x19},
    {"f4", 0x1A},
    {"f5", 0x1B},
    {"f6", 0x1C},
    {"f7", 0x1D},
    {"f8", 0x1E},
    {"f9", 0x1F},
    {"f10", 0x20},
    {"f11", 0x21},
    {"f12", 0x22},
    {"f13", 0x23},
    {"f14", 0x24},
    {"f15", 0x25},
};

namespace Asm {
const std::map<std::string, uint8_t> INSTR_NAMES {
    {"nop", 0},
    {"push", 1},
    {"pop", 2},
    {"load", 3},
    {"loadf", 4},
    {"store", 5},
    {"storef", 6},
    {"copy", 7},
    {"copyf", 8},
    {"exit", 9},
    {"call", 10},
    {"ret", 11},
    {"sys", 12},
    {"lea", 13},
    {"add", 14},
    {"addf", 15},
    {"sub", 16},
    {"subf", 17},
    {"mul", 18},
    {"mulf", 19},
    {"div", 20},
    {"divf", 21},
    {"sqrt", 22},
    {"and", 23},
    {"or", 24},
    {"xor", 25},
    {"not", 26},
    {"lsh", 27},
    {"rsh", 28},
    {"srsh", 29},
    {"b2l", 30},
    {"s2l", 31},
    {"i2l", 32},
    {"b2sl", 33},
    {"s2sl", 34},
    {"i2sl", 35},
    {"f2d", 36},
    {"d2f", 37},
    {"i2f", 38},
    {"i2d", 39},
    {"f2i", 40},
    {"d2i", 41},
    {"cmp", 42},
    {"cmpf", 43},
    {"jmp", 44},
    {"je", 45},
    {"jne", 46},
    {"jgt", 47},
    {"jlt", 48},
    {"jge", 49},
    {"jle", 50},
};

const std::array<std::vector<InstrParamList>, 51> INSTR_ASM_DEFS {
    std::vector<InstrParamList>{
        InstrParamList{
            0xA0,
            0,
            {
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0x01,
            INSTR_FLAG_TYPE_VARIANTS,
            {
                InstrParamType::INT_TYPE, InstrParamType::INT_NUM,
            },
            {
                {UVM_TYPE_I8, 0x01},
                {UVM_TYPE_I16, 0x02},
                {UVM_TYPE_I32, 0x03},
                {UVM_TYPE_I64, 0x04},
            },
        },
        InstrParamList{
            0x05,
            INSTR_FLAG_ENCODE_TYPE,
            {
                InstrParamType::INT_TYPE, InstrParamType::INT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0x06,
            INSTR_FLAG_ENCODE_TYPE,
            {
                InstrParamType::INT_TYPE,
            },
            {
            },
        },
        InstrParamList{
            0x07,
            INSTR_FLAG_ENCODE_TYPE,
            {
                InstrParamType::INT_TYPE, InstrParamType::INT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0x11,
            INSTR_FLAG_TYPE_VARIANTS,
            {
                InstrParamType::INT_TYPE, InstrParamType::INT_NUM, InstrParamType::INT_REG,
            },
            {
                {UVM_TYPE_I8, 0x11},
                {UVM_TYPE_I16, 0x12},
                {UVM_TYPE_I32, 0x13},
                {UVM_TYPE_I64, 0x14},
            },
        },
        InstrParamList{
            0x15,
            INSTR_FLAG_ENCODE_TYPE,
            {
                InstrParamType::INT_TYPE, InstrParamType::REG_OFFSET, InstrParamType::INT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0x16,
            INSTR_FLAG_TYPE_VARIANTS,
            {
                InstrParamType::FLOAT_TYPE, InstrParamType::FLOAT_NUM, InstrParamType::FLOAT_REG,
            },
            {
                {UVM_TYPE_F32, 0x16},
                {UVM_TYPE_F64, 0x17},
            },
        },
        InstrParamList{
            0x18,
            INSTR_FLAG_ENCODE_TYPE,
            {
                InstrParamType::FLOAT_TYPE, InstrParamType::REG_OFFSET, InstrParamType::FLOAT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0x08,
            INSTR_FLAG_ENCODE_TYPE,
            {
                InstrParamType::INT_TYPE, InstrParamType::INT_REG, InstrParamType::REG_OFFSET,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0x09,
            INSTR_FLAG_ENCODE_TYPE,
            {
                InstrParamType::FLOAT_TYPE, InstrParamType::FLOAT_REG, InstrParamType::REG_OFFSET,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0x21,
            INSTR_FLAG_TYPE_VARIANTS,
            {
                InstrParamType::INT_TYPE, InstrParamType::INT_NUM, InstrParamType::REG_OFFSET,
            },
            {
                {UVM_TYPE_I8, 0x21},
                {UVM_TYPE_I16, 0x22},
                {UVM_TYPE_I32, 0x23},
                {UVM_TYPE_I64, 0x24},
            },
        },
        InstrParamList{
            0x25,
            INSTR_FLAG_ENCODE_TYPE,
            {
                InstrParamType::INT_TYPE, InstrParamType::INT_REG, InstrParamType::INT_REG,
            },
            {
            },
        },
        InstrParamList{
            0x26,
            INSTR_FLAG_ENCODE_TYPE,
            {
                InstrParamType::INT_TYPE, InstrParamType::REG_OFFSET, InstrParamType::REG_OFFSET,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0x27,
            INSTR_FLAG_TYPE_VARIANTS,
            {
                InstrParamType::FLOAT_TYPE, InstrParamType::FLOAT_NUM, InstrParamType::REG_OFFSET,
            },
            {
                {UVM_TYPE_F32, 0x27},
                {UVM_TYPE_F64, 0x28},
            },
        },
        InstrParamList{
            0x29,
            INSTR_FLAG_ENCODE_TYPE,
            {
                InstrParamType::FLOAT_TYPE, InstrParamType::FLOAT_REG, InstrParamType::FLOAT_REG,
            },
            {
            },
        },
        InstrParamList{
            0x2A,
            INSTR_FLAG_ENCODE_TYPE,
            {
                InstrParamType::FLOAT_TYPE, InstrParamType::REG_OFFSET, InstrParamType::REG_OFFSET,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0x50,
            0,
            {
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0x20,
            0,
            {
                InstrParamType::LABEL_ID,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0x30,
            0,
            {
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0x40,
            0,
            {
                InstrParamType::SYS_INT,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0x10,
            0,
            {
                InstrParamType::REG_OFFSET, InstrParamType::INT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0x31,
            INSTR_FLAG_TYPE_VARIANTS,
            {
                InstrParamType::INT_TYPE, InstrParamType::INT_REG, InstrParamType::INT_NUM,
            },
            {
                {UVM_TYPE_I8, 0x31},
                {UVM_TYPE_I16, 0x32},
                {UVM_TYPE_I32, 0x33},
                {UVM_TYPE_I64, 0x34},
            },
        },
        InstrParamList{
            0x35,
            INSTR_FLAG_ENCODE_TYPE,
            {
                InstrParamType::INT_TYPE, InstrParamType::INT_REG, InstrParamType::INT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0x36,
            INSTR_FLAG_TYPE_VARIANTS,
            {
                InstrParamType::FLOAT_TYPE, InstrParamType::FLOAT_REG, InstrParamType::FLOAT_NUM,
            },
            {
                {UVM_TYPE_I32, 0x37},
                {UVM_TYPE_I64, 0x38},
            },
        },
        InstrParamList{
            0x38,
            INSTR_FLAG_ENCODE_TYPE,
            {
                InstrParamType::FLOAT_TYPE, InstrParamType::FLOAT_REG, InstrParamType::FLOAT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0x41,
            INSTR_FLAG_TYPE_VARIANTS,
            {
                InstrParamType::INT_TYPE, InstrParamType::INT_REG, InstrParamType::INT_NUM,
            },
            {
                {UVM_TYPE_I8, 0x41},
                {UVM_TYPE_I16, 0x42},
                {UVM_TYPE_I32, 0x43},
                {UVM_TYPE_I64, 0x44},
            },
        },
        InstrParamList{
            0x45,
            INSTR_FLAG_ENCODE_TYPE,
            {
                InstrParamType::INT_TYPE, InstrParamType::INT_REG, InstrParamType::INT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0x46,
            INSTR_FLAG_TYPE_VARIANTS,
            {
                InstrParamType::FLOAT_TYPE, InstrParamType::FLOAT_REG, InstrParamType::FLOAT_NUM,
            },
            {
                {UVM_TYPE_I32, 0x46},
                {UVM_TYPE_I64, 0x47},
            },
        },
        InstrParamList{
            0x48,
            INSTR_FLAG_ENCODE_TYPE,
            {
                InstrParamType::FLOAT_TYPE, InstrParamType::FLOAT_REG, InstrParamType::FLOAT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0x51,
            INSTR_FLAG_TYPE_VARIANTS,
            {
                InstrParamType::INT_TYPE, InstrParamType::INT_REG, InstrParamType::INT_NUM,
            },
            {
                {UVM_TYPE_I8, 0x51},
                {UVM_TYPE_I16, 0x52},
                {UVM_TYPE_I32, 0x53},
                {UVM_TYPE_I64, 0x54},
            },
        },
        InstrParamList{
            0x55,
            INSTR_FLAG_ENCODE_TYPE,
            {
                InstrParamType::INT_TYPE, InstrParamType::INT_REG, InstrParamType::INT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0x56,
            INSTR_FLAG_TYPE_VARIANTS,
            {
                InstrParamType::FLOAT_TYPE, InstrParamType::FLOAT_REG, InstrParamType::FLOAT_NUM,
            },
            {
                {UVM_TYPE_I32, 0x56},
                {UVM_TYPE_I64, 0x57},
            },
        },
        InstrParamList{
            0x58,
            INSTR_FLAG_ENCODE_TYPE,
            {
                InstrParamType::FLOAT_TYPE, InstrParamType::FLOAT_REG, InstrParamType::FLOAT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0x61,
            INSTR_FLAG_TYPE_VARIANTS,
            {
                InstrParamType::INT_TYPE, InstrParamType::INT_REG, InstrParamType::INT_NUM,
            },
            {
                {UVM_TYPE_I8, 0x61},
                {UVM_TYPE_I16, 0x62},
                {UVM_TYPE_I32, 0x63},
                {UVM_TYPE_I64, 0x64},
            },
        },
        InstrParamList{
            0x65,
            INSTR_FLAG_ENCODE_TYPE,
            {
                InstrParamType::INT_TYPE, InstrParamType::INT_REG, InstrParamType::INT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0x66,
            INSTR_FLAG_TYPE_VARIANTS,
            {
                InstrParamType::FLOAT_TYPE, InstrParamType::FLOAT_REG, InstrParamType::FLOAT_NUM,
            },
            {
                {UVM_TYPE_I32, 0x66},
                {UVM_TYPE_I64, 0x67},
            },
        },
        InstrParamList{
            0x68,
            INSTR_FLAG_ENCODE_TYPE,
            {
                InstrParamType::FLOAT_TYPE, InstrParamType::FLOAT_REG, InstrParamType::FLOAT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0x86,
            INSTR_FLAG_ENCODE_TYPE,
            {
                InstrParamType::FLOAT_TYPE, InstrParamType::FLOAT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0x71,
            INSTR_FLAG_TYPE_VARIANTS,
            {
                InstrParamType::INT_TYPE, InstrParamType::INT_NUM, InstrParamType::INT_REG,
            },
            {
                {UVM_TYPE_I8, 0x71},
                {UVM_TYPE_I16, 0x72},
                {UVM_TYPE_I32, 0x73},
                {UVM_TYPE_I64, 0x74},
            },
        },
        InstrParamList{
            0x75,
            INSTR_FLAG_ENCODE_TYPE,
            {
                InstrParamType::INT_TYPE, InstrParamType::INT_REG, InstrParamType::INT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0x81,
            INSTR_FLAG_TYPE_VARIANTS,
            {
                InstrParamType::INT_TYPE, InstrParamType::INT_NUM, InstrParamType::INT_REG,
            },
            {
                {UVM_TYPE_I8, 0x81},
                {UVM_TYPE_I16, 0x82},
                {UVM_TYPE_I32, 0x83},
                {UVM_TYPE_I64, 0x84},
            },
        },
        InstrParamList{
            0x85,
            INSTR_FLAG_ENCODE_TYPE,
            {
                InstrParamType::INT_TYPE, InstrParamType::INT_REG, InstrParamType::INT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0x91,
            INSTR_FLAG_TYPE_VARIANTS,
            {
                InstrParamType::INT_TYPE, InstrParamType::INT_NUM, InstrParamType::INT_REG,
            },
            {
                {UVM_TYPE_I8, 0x91},
                {UVM_TYPE_I16, 0x92},
                {UVM_TYPE_I32, 0x93},
                {UVM_TYPE_I64, 0x94},
            },
        },
        InstrParamList{
            0x95,
            INSTR_FLAG_ENCODE_TYPE,
            {
                InstrParamType::INT_TYPE, InstrParamType::INT_REG, InstrParamType::INT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0xA1,
            INSTR_FLAG_TYPE_VARIANTS,
            {
                InstrParamType::INT_TYPE, InstrParamType::INT_NUM, InstrParamType::INT_REG,
            },
            {
                {UVM_TYPE_I8, 0xA1},
                {UVM_TYPE_I16, 0xA2},
                {UVM_TYPE_I32, 0xA3},
                {UVM_TYPE_I64, 0xA4},
            },
        },
        InstrParamList{
            0xA5,
            INSTR_FLAG_ENCODE_TYPE,
            {
                InstrParamType::INT_TYPE, InstrParamType::INT_REG, InstrParamType::INT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0x76,
            0,
            {
                InstrParamType::INT_NUM, InstrParamType::INT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0x77,
            0,
            {
                InstrParamType::INT_NUM, InstrParamType::INT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0x78,
            0,
            {
                InstrParamType::INT_NUM, InstrParamType::INT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0xB1,
            0,
            {
                InstrParamType::INT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0xB2,
            0,
            {
                InstrParamType::INT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0xB3,
            0,
            {
                InstrParamType::INT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0xC1,
            0,
            {
                InstrParamType::INT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0xC2,
            0,
            {
                InstrParamType::INT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0xC3,
            0,
            {
                InstrParamType::INT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0xB4,
            0,
            {
                InstrParamType::FLOAT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0xC4,
            0,
            {
                InstrParamType::FLOAT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0xB2,
            0,
            {
                InstrParamType::INT_REG, InstrParamType::FLOAT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0xC5,
            0,
            {
                InstrParamType::INT_REG, InstrParamType::FLOAT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0xB6,
            0,
            {
                InstrParamType::FLOAT_REG, InstrParamType::INT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0xC6,
            0,
            {
                InstrParamType::FLOAT_REG, InstrParamType::INT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0xD1,
            INSTR_FLAG_ENCODE_TYPE,
            {
                InstrParamType::INT_TYPE, InstrParamType::INT_REG, InstrParamType::INT_REG,
            },
            {
            },
        },
        InstrParamList{
            0xD2,
            INSTR_FLAG_ENCODE_TYPE,
            {
                InstrParamType::INT_TYPE, InstrParamType::INT_REG, InstrParamType::REG_OFFSET,
            },
            {
            },
        },
        InstrParamList{
            0xD3,
            INSTR_FLAG_ENCODE_TYPE,
            {
                InstrParamType::INT_TYPE, InstrParamType::REG_OFFSET, InstrParamType::REG_OFFSET,
            },
            {
            },
        },
        InstrParamList{
            0xD4,
            INSTR_FLAG_ENCODE_TYPE,
            {
                InstrParamType::INT_TYPE, InstrParamType::REG_OFFSET, InstrParamType::INT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0xD5,
            0,
            {
                InstrParamType::FLOAT_TYPE, InstrParamType::FLOAT_REG, InstrParamType::FLOAT_REG,
            },
            {
            },
        },
        InstrParamList{
            0xD6,
            0,
            {
                InstrParamType::FLOAT_TYPE, InstrParamType::FLOAT_REG, InstrParamType::REG_OFFSET,
            },
            {
            },
        },
        InstrParamList{
            0xD7,
            0,
            {
                InstrParamType::FLOAT_TYPE, InstrParamType::REG_OFFSET, InstrParamType::REG_OFFSET,
            },
            {
            },
        },
        InstrParamList{
            0xD8,
            0,
            {
                InstrParamType::FLOAT_TYPE, InstrParamType::REG_OFFSET, InstrParamType::FLOAT_REG,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0xE1,
            0,
            {
                InstrParamType::LABEL_ID,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0xE2,
            0,
            {
                InstrParamType::LABEL_ID,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0xE3,
            0,
            {
                InstrParamType::LABEL_ID,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0xE4,
            0,
            {
                InstrParamType::LABEL_ID,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0xE5,
            0,
            {
                InstrParamType::LABEL_ID,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0xE6,
            0,
            {
                InstrParamType::LABEL_ID,
            },
            {
            },
        },
    },
    std::vector<InstrParamList>{
        InstrParamList{
            0xE7,
            0,
            {
                InstrParamType::LABEL_ID,
            },
            {
            },
        },
    },
};

} // namespace Asm
