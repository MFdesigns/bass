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
#include <cstdint>
#include <vector>

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
    uint8_t Instr;
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
