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

#include "assembler.h"
#include "source_file.h"
#include <filesystem>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "usage: uvm-assembler <path>\n";
        return -1;
    }

    std::filesystem::path p{argv[1]};
    if (!std::filesystem::exists(p)) {
        std::cout << "Error: source file does not exist\n";
        return -1;
    }

    uint8_t* source = nullptr;
    uint32_t size = 0;
    bool success = readSource(p, &source, size);
    if (!success) {
        std::cout << "Error: could not read source file\n";
        return -1;
    }

    Assembler asmr{source, size};
    bool tokSuccess = asmr.tokenize();
    if (!tokSuccess) {
        return -1;
    }

    bool astSuccess = asmr.buildAST();
    if (!astSuccess) {
        return -1;
    }
}
