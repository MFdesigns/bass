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

// This script generates a encoding header file from encoding json data

// To run this file:
// deno run --unstable --allow-read --allow-write ./src/asm/encodingData.js

import { readJson } from "https://deno.land/std/fs/mod.ts";

const DIRNAME = './src/asm/';
const IN_FILE_NAME = 'encodingData.json';
const OUT_FILE_NAME = 'encoding.hpp';
const TAB = '    ';

const HEADER =
    `// ======================================================================== //
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
`;
const UVM_TYPES = {
    'i8': 'UVM_TYPE_I8',
    'i16': 'UVM_TYPE_I16',
    'i32': 'UVM_TYPE_I32',
    'i64': 'UVM_TYPE_I64',
    'f32': 'UVM_TYPE_F32',
    'f64': 'UVM_TYPE_F64',
};

const PARAM_TYPES = {
    'iT': 'INT_TYPE',
    'fT': 'FLOAT_TYPE',
    'iReg': 'INT_REG',
    'fReg': 'FLOAT_REG',
    'function': 'FUNC_ID',
    'label': 'LABEL_ID',
    'RO': 'REG_OFFSET',
    'int': 'INT_NUM',
    'float': 'FLOAT_NUM',
    'sysID': 'SYS_INT',
};

/**
 * Converts JSON format param type to C++ param type
 * @param {string} param
 */
function toCPPParamType(param) {
    return `InstrParamType::${PARAM_TYPES[param]}`;
}

/**
 * Converts JSON format type to UVM type
 * @param {string} type
 */
function toUVMType(type) {
    return UVM_TYPES[type];
}

/**
 *
 * @param {number} count Amount of tabs
 */
function tab(count) {
    let buff = '';
    for (let i = 0; i < count; i++) {
        buff += TAB;
    }
    return buff;
}

/**
 * Generates the register lookup table
 * @param {*} data JSON Data
 * @return {string} generated C++ code
 */
function generateRegisterLookupTable(data) {
    let buffer = 'const std::map<std::string, uint8_t> ASM_REGISTERS {\n';

    data.registers.forEach((reg) => {
        buffer += `${tab(1)}{"${reg.name}", ${reg.bytecode}},\n`;
    });

    buffer += '};\n\n';

    return buffer;
}

/**
 *
 * @param {*} data JSON data from file
 */
function generateHeaderFile(data) {
    // Create new output buffer and insert .hpp header
    let buffer = `${HEADER}\n`;

    buffer += generateRegisterLookupTable(data);

    // Add namespace
    buffer += 'namespace Asm {\n';

    // Generate instruction name lookup table
    buffer += 'const std::map<std::string, uint8_t> INSTR_NAMES {\n';
    data.instructions.forEach((instr, i) => {
        buffer += `${tab(1)}{"${instr.name}", ${i}},\n`;
    });
    // Closing brace of 'constexpr std::map<const char*, uint8_t> INSTR_NAMES {'
    buffer += '};\n\n';

    // Generate instruction encoding resolution
    buffer += `const std::array<std::vector<InstrParamList>, ${data.instructions.length}> INSTR_ASM_DEFS {\n`
    data.instructions.forEach((instr, i) => {
        buffer += `${tab(1)}std::vector<InstrParamList>{\n`;

        // Instruction paramlist array
        instr.paramList.forEach((paramList) => {
            buffer += `${tab(2)}InstrParamList{\n`;
            // Instruction param list content
            buffer += `${tab(3)}${paramList.opcode},\n`;

            if (paramList.encodeType && paramList.typeVariants.length > 0) {
                buffer += `${tab(3)}INSTR_FLAG_ENCODE_TYPE | INSTR_FLAG_TYPE_VARIANTS,\n`;
            } else if (paramList.encodeType) {
                buffer += `${tab(3)}INSTR_FLAG_ENCODE_TYPE,\n`;
            } else if (paramList.typeVariants.length > 0) {
                buffer += `${tab(3)}INSTR_FLAG_TYPE_VARIANTS,\n`;
            } else {
                buffer += `${tab(3)}0,\n`;
            }

            // Instruction params
            buffer += `${tab(3)}{\n`;
            paramList.params.forEach((param, i) => {
                if (i === 0) {
                    buffer += tab(4);
                }
                buffer += `${toCPPParamType(param)}, `;
                if (i + 1 === paramList.params.length) {
                    buffer += '\n';
                }
            });
            buffer += `${tab(3)}},\n`;

            // Instruction type variants
            buffer += `${tab(3)}{\n`;
            paramList.typeVariants.forEach((variant) => {
                buffer += `${tab(4)}{${toUVMType(variant.type)}, ${variant.opcode}},\n`;
            });
            buffer += `${tab(3)}},\n`;

            buffer += `${tab(2)}},\n`;
        });
        buffer += `${tab(1)}},\n`;
    });
    // Closing brace of 'constexpr std::array<InstrParamList> INSTR_ASM_DEFS {\n'
    buffer += '};\n\n';
    buffer += '} // namespace Asm\n'; // Namespace closing bracket

    return buffer;
}

(async function main() {
    const data = await readJson(`${DIRNAME}${IN_FILE_NAME}`);
    const content = generateHeaderFile(data);
    await Deno.writeTextFile(`${DIRNAME}${OUT_FILE_NAME}`, content);
}())
