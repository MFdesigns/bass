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

#include "assembler.hpp"
#include <cstdint>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "usage: uvm-assembler <path>\n";
        return -1;
    }

    Assembler asmler{};

    bool fileReadSucc = asmler.readSource(argv[1]);
    if (!fileReadSucc) {
        std::cout << "[ERROR] Could not read source file '" << argv[1] << "'\n";
        return -1;
    }

    bool status = asmler.assemble();
    if (!status) {
        std::cout << "Assembler exited with an error\n";
        return -1;
    }
}
