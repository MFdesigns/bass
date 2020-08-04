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
#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

enum class Instructions {
    NOP,
    PUSH,
    POP,
    LOAD,
    LOADF,
    STORE,
    STOREF,
    COPY,
    COPYF,
    EXIT,
    CALL,
    RET,
    SYS,
    LEA,
    ADD,
    ADDF,
    SUB,
    SUBF,
    MUL,
    MULF,
    DIV,
    DIVF,
    SQRT,
    AND,
    OR,
    XOR,
    NOT,
    LSH,
    RSH,
    SRSH,
    B2L,
    S2L,
    I2L,
    B2SL,
    S2SL,
    I2SL,
    F2D,
    D2F,
    I2F,
    I2D,
    F2I,
    D2I,
    CMP,
    CMPF,
    JMP,
    JE,
    JNE,
    JGT,
    JLT,
    JGE,
    JLE,
};

// Opcode definitions
constexpr uint8_t OP_NOP = 0x00;
constexpr uint8_t OP_PUSH_I8 = 0x01;
constexpr uint8_t OP_PUSH_I16 = 0x02;
constexpr uint8_t OP_PUSH_I32 = 0x03;
constexpr uint8_t OP_PUSH_I64 = 0x04;
constexpr uint8_t OP_PUSH_IT_IR = 0x05;
constexpr uint8_t OP_POP_IT = 0x06;
constexpr uint8_t OP_POP_IT_IR = 0x07;
constexpr uint8_t OP_LOAD_I8_IR = 0x11;
constexpr uint8_t OP_LOAD_I16_IR = 0x12;
constexpr uint8_t OP_LOAD_I32_IR = 0x13;
constexpr uint8_t OP_LOAD_I64_IR = 0x14;
constexpr uint8_t OP_LOAD_IT_RO_IR = 0x15;
constexpr uint8_t OP_LOADF_F32_FR = 0x16;
constexpr uint8_t OP_LOADF_F64_FR = 0x17;
constexpr uint8_t OP_LOADF_FT_RO_FR = 0x18;
constexpr uint8_t OP_STORE_IT_IR_RO = 0x08;
constexpr uint8_t OP_STOREF_FT_FR_RO = 0x09;
constexpr uint8_t OP_COPY_I8_RO = 0x21;
constexpr uint8_t OP_COPY_I16_RO = 0x22;
constexpr uint8_t OP_COPY_I32_RO = 0x23;
constexpr uint8_t OP_COPY_I64_RO = 0x24;
constexpr uint8_t OP_COPY_IT_IR_IR = 0x25;
constexpr uint8_t OP_COPY_IT_RO_RO = 0x26;
constexpr uint8_t OP_COPYF_F32_RO = 0x27;
constexpr uint8_t OP_COPYF_F64_RO = 0x28;
constexpr uint8_t OP_COPYF_FT_FR_FR = 0x29;
constexpr uint8_t OP_COPYF_FT_RO_RO = 0x2A;
constexpr uint8_t OP_EXIT = 0x50;
constexpr uint8_t OP_CALL_VADDR = 0x20;
constexpr uint8_t OP_RET = 0x30;
constexpr uint8_t OP_SYS_I8 = 0x40;
constexpr uint8_t OP_LEA_RO_IR = 0x10;
constexpr uint8_t OP_ADD_I8_IR = 0x31;
constexpr uint8_t OP_ADD_I16_IR = 0x32;
constexpr uint8_t OP_ADD_I32_IR = 0x33;
constexpr uint8_t OP_ADD_I64_IR = 0x34;
constexpr uint8_t OP_ADD_IT_IR_IR = 0x35;
constexpr uint8_t OP_ADDF_F32_FR = 0x36;
constexpr uint8_t OP_ADDF_F64_FR = 0x37;
constexpr uint8_t OP_ADDF_FT_FR_FR = 0x38;
constexpr uint8_t OP_SUB_I8_IR = 0x41;
constexpr uint8_t OP_SUB_I16_IR = 0x42;
constexpr uint8_t OP_SUB_I32_IR = 0x43;
constexpr uint8_t OP_SUB_I64_IR = 0x44;
constexpr uint8_t OP_SUB_IT_IR_IR = 0x45;
constexpr uint8_t OP_SUBF_F32_FR = 0x46;
constexpr uint8_t OP_SUBF_F64_FR = 0x47;
constexpr uint8_t OP_SUBF_FT_FR_FR = 0x48;
constexpr uint8_t OP_MUL_I8_IR = 0x51;
constexpr uint8_t OP_MUL_I16_IR = 0x52;
constexpr uint8_t OP_MUL_I32_IR = 0x53;
constexpr uint8_t OP_MUL_I64_IR = 0x54;
constexpr uint8_t OP_MUL_IT_IR_IR = 0x55;
constexpr uint8_t OP_MULF_F32_FR = 0x56;
constexpr uint8_t OP_MULF_F64_FR = 0x57;
constexpr uint8_t OP_MULF_FT_FR_FR = 0x58;
constexpr uint8_t OP_DIV_I8_IR = 0x61;
constexpr uint8_t OP_DIV_I16_IR = 0x62;
constexpr uint8_t OP_DIV_I32_IR = 0x63;
constexpr uint8_t OP_DIV_I64_IR = 0x64;
constexpr uint8_t OP_DIV_IT_IR_IR = 0x65;
constexpr uint8_t OP_DIVF_F32_FR = 0x66;
constexpr uint8_t OP_DIVF_F64_FR = 0x67;
constexpr uint8_t OP_DIVF_FT_FR_FR = 0x68;
constexpr uint8_t OP_SQRT_FT_FR = 0x86;
constexpr uint8_t OP_AND_I8_IR = 0x71;
constexpr uint8_t OP_AND_I16_IR = 0x72;
constexpr uint8_t OP_AND_I32_IR = 0x73;
constexpr uint8_t OP_AND_I64_IR = 0x74;
constexpr uint8_t OP_AND_IT_IR_IR = 0x75;
constexpr uint8_t OP_OR_I8_IR = 0x81;
constexpr uint8_t OP_OR_I16_IR = 0x82;
constexpr uint8_t OP_OR_I32_IR = 0x83;
constexpr uint8_t OP_OR_I64_IR = 0x84;
constexpr uint8_t OP_OR_IT_IR_IR = 0x85;
constexpr uint8_t OP_XOR_I8_IR = 0x91;
constexpr uint8_t OP_XOR_I16_IR = 0x92;
constexpr uint8_t OP_XOR_I32_IR = 0x93;
constexpr uint8_t OP_XOR_I64_IR = 0x94;
constexpr uint8_t OP_XOR_IT_IR_IR = 0x95;
constexpr uint8_t OP_NOT_I8_IR = 0xA1;
constexpr uint8_t OP_NOT_I16_IR = 0xA2;
constexpr uint8_t OP_NOT_I32_IR = 0xA3;
constexpr uint8_t OP_NOT_I64_IR = 0xA4;
constexpr uint8_t OP_NOT_IT_IR_IR = 0xA5;
constexpr uint8_t OP_LSH_I8_IR = 0x76;
constexpr uint8_t OP_RSH_I8_IR = 0x77;
constexpr uint8_t OP_SRSH_I8_IR = 0x78;
constexpr uint8_t OP_B2L_IR = 0xB1;
constexpr uint8_t OP_S2L_IR = 0xB2;
constexpr uint8_t OP_I2L_IR = 0xB3;
constexpr uint8_t OP_B2SL_IR = 0xC1;
constexpr uint8_t OP_S2SL_IR = 0xC2;
constexpr uint8_t OP_I2SL_IR = 0xC3;
constexpr uint8_t OP_F2D_FR = 0xB4;
constexpr uint8_t OP_D3F_FR = 0xC4;
constexpr uint8_t OP_I2F_IR_FR = 0xB5;
constexpr uint8_t OP_I2D_IR_FR = 0xC5;
constexpr uint8_t OP_F2I_FR_IR = 0xB6;
constexpr uint8_t OP_D2I_FR_IR = 0xC6;
constexpr uint8_t OP_CMP_IT_IR_IR = 0xD1;
constexpr uint8_t OP_CMP_IT_IR_RO = 0xD2;
constexpr uint8_t OP_CMP_IT_RO_RO = 0xD3;
constexpr uint8_t OP_CMP_IT_RO_IR = 0xD4;
constexpr uint8_t OP_CMPF_FT_FR_FR = 0xD5;
constexpr uint8_t OP_CMPF_FT_FR_RO = 0xD6;
constexpr uint8_t OP_CMPF_FT_RO_RO = 0xD7;
constexpr uint8_t OP_CMPF_FT_RO_FR = 0xD8;
constexpr uint8_t OP_JMP_VADDR = 0xE1;
constexpr uint8_t OP_JE_VADDR = 0xE2;
constexpr uint8_t OP_JNE_VADDR = 0xE3;
constexpr uint8_t OP_JGT_VADDR = 0xE4;
constexpr uint8_t OP_JLT_VADDR = 0xE5;
constexpr uint8_t OP_JGE_VADDR = 0xE6;
constexpr uint8_t OP_JLE_VADDR = 0xE7;

// UVM type definitions
constexpr uint8_t UVM_TYPE_I8 = 0x01;
constexpr uint8_t UVM_TYPE_I16 = 0x02;
constexpr uint8_t UVM_TYPE_I32 = 0x03;
constexpr uint8_t UVM_TYPE_I64 = 0x04;
constexpr uint8_t UVM_TYPE_F32 = 0xF0;
constexpr uint8_t UVM_TYPE_F64 = 0xF1;

// Flags that are used to encode the instructions
constexpr uint8_t INSTR_FLAG_ENCODE_TYPE =
    0b0000'0001; // Type should be encoded into bytecode
constexpr uint8_t INSTR_FLAG_TYPE_VARIANTS =
    0b0000'0010; // Instruction has opcode variants depending on the uvm type

enum class InstrParamType {
    INT_TYPE,
    FLOAT_TYPE,
    FUNC_ID,
    LABEL_ID,
    INT_REG,
    FLOAT_REG,
    REG_OFFSET,
    INT_NUM,
    FLOAT_NUM,
    SYS_INT,
};

struct InstrNameDef {
    const char* Str;
    Instructions Instr;
};

const static std::array<InstrNameDef, 12> INSTR_NAMES = {
    InstrNameDef{"nop", Instructions::NOP},
    InstrNameDef{"push", Instructions::PUSH},
    InstrNameDef{"pop", Instructions::POP},
    InstrNameDef{"load", Instructions::LOAD},
    InstrNameDef{"store", Instructions::STORE},
    InstrNameDef{"storef", Instructions::STOREF},
    InstrNameDef{"copy", Instructions::COPY},
    InstrNameDef{"exit", Instructions::EXIT},
    InstrNameDef{"call", Instructions::CALL},
    InstrNameDef{"ret", Instructions::RET},
    InstrNameDef{"sys", Instructions::SYS},
    InstrNameDef{"add", Instructions::ADD},
};

struct TypeVariant {
    uint8_t Type;
    uint8_t Opcode;
};

struct InstrParamList {
    uint8_t Opcode;
    uint8_t Flags;
    std::vector<InstrParamType> Params;
    std::vector<TypeVariant> OpcodeVariants;
};

// Contains all instruction parameter signatures and opcodes needed to type
// check and encode instructions
const static std::map<Instructions, std::vector<InstrParamList>> INSTR_ASM_DEFS{
    {Instructions::NOP, {InstrParamList{OP_NOP, 0, {}, {}}}},
    {Instructions::PUSH,
     {
         InstrParamList{0,
                        INSTR_FLAG_TYPE_VARIANTS,
                        {InstrParamType::INT_TYPE, InstrParamType::INT_NUM},
                        {
                            {UVM_TYPE_I8, OP_PUSH_I8},
                            {UVM_TYPE_I16, OP_PUSH_I16},
                            {UVM_TYPE_I32, OP_PUSH_I32},
                            {UVM_TYPE_I64, OP_PUSH_I64},
                        }},
         InstrParamList{OP_PUSH_IT_IR,
                        INSTR_FLAG_ENCODE_TYPE,
                        {InstrParamType::INT_TYPE, InstrParamType::INT_REG},
                        {}},
     }},
    {Instructions::POP,
     {
         InstrParamList{
             OP_POP_IT, INSTR_FLAG_ENCODE_TYPE, {InstrParamType::INT_TYPE}, {}},
         InstrParamList{OP_POP_IT_IR,
                        INSTR_FLAG_ENCODE_TYPE,
                        {InstrParamType::INT_TYPE, InstrParamType::INT_REG},
                        {}},
     }},
    {Instructions::LOAD,
     {
         InstrParamList{0,
                        INSTR_FLAG_TYPE_VARIANTS,
                        {InstrParamType::INT_TYPE, InstrParamType::INT_NUM,
                         InstrParamType::INT_REG},
                        {
                            {UVM_TYPE_I8, OP_LOAD_I8_IR},
                            {UVM_TYPE_I16, OP_LOAD_I16_IR},
                            {UVM_TYPE_I32, OP_LOAD_I32_IR},
                            {UVM_TYPE_I64, OP_LOAD_I64_IR},
                        }},
         InstrParamList{OP_LOAD_IT_RO_IR,
                        INSTR_FLAG_ENCODE_TYPE,
                        {InstrParamType::INT_TYPE, InstrParamType::REG_OFFSET,
                         InstrParamType::INT_REG},
                        {}},
     }},
    {Instructions::STORE,
     {
         InstrParamList{OP_STORE_IT_IR_RO,
                        INSTR_FLAG_ENCODE_TYPE,
                        {InstrParamType::INT_TYPE, InstrParamType::INT_REG,
                         InstrParamType::REG_OFFSET},
                        {}},
     }},
    {Instructions::STOREF,
     {
         InstrParamList{OP_STOREF_FT_FR_RO,
                        INSTR_FLAG_ENCODE_TYPE,
                        {InstrParamType::FLOAT_TYPE, InstrParamType::FLOAT_REG,
                         InstrParamType::REG_OFFSET},
                        {}},
     }},
    {Instructions::COPY,
     {
         InstrParamList{0,
                        INSTR_FLAG_TYPE_VARIANTS,
                        {InstrParamType::INT_TYPE, InstrParamType::INT_NUM,
                         InstrParamType::REG_OFFSET},
                        {
                            {UVM_TYPE_I8, OP_COPY_I8_RO},
                            {UVM_TYPE_I16, OP_COPY_I16_RO},
                            {UVM_TYPE_I32, OP_COPY_I32_RO},
                            {UVM_TYPE_I64, OP_COPY_I64_RO},
                        }},
         InstrParamList{OP_COPY_IT_IR_IR,
                        INSTR_FLAG_ENCODE_TYPE,
                        {InstrParamType::INT_TYPE, InstrParamType::INT_REG,
                         InstrParamType::INT_REG},
                        {}},
         InstrParamList{OP_COPY_IT_RO_RO,
                        INSTR_FLAG_ENCODE_TYPE,
                        {InstrParamType::INT_TYPE, InstrParamType::REG_OFFSET,
                         InstrParamType::REG_OFFSET},
                        {}},
     }},
    {Instructions::EXIT, {InstrParamList{OP_EXIT, 0, {}, {}}}},
    {Instructions::CALL,
     {InstrParamList{OP_CALL_VADDR, 0, {InstrParamType::FUNC_ID}, {}}}},
    {Instructions::RET, {InstrParamList{OP_RET, 0, {}, {}}}},
    {Instructions::SYS,
     {InstrParamList{OP_SYS_I8, 0, {InstrParamType::SYS_INT}, {}}}},
    {Instructions::ADD,
     {
         InstrParamList{0,
                        INSTR_FLAG_TYPE_VARIANTS,
                        {InstrParamType::INT_TYPE, InstrParamType::INT_NUM,
                         InstrParamType::INT_REG},
                        {
                            {UVM_TYPE_I8, OP_ADD_I8_IR},
                            {UVM_TYPE_I16, OP_ADD_I16_IR},
                            {UVM_TYPE_I32, OP_ADD_I32_IR},
                            {UVM_TYPE_I64, OP_ADD_I64_IR},
                        }},
         InstrParamList{OP_ADD_IT_IR_IR,
                        INSTR_FLAG_ENCODE_TYPE,
                        {InstrParamType::INT_TYPE, InstrParamType::INT_REG,
                         InstrParamType::INT_REG},
                        {}},
     }},
};
