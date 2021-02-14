/**
 * Copyright 2020-2021 Michel Fäh
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

#include "asm/asm.hpp"
#include "assembler.hpp"
#include <cstdint>
#include <iostream>
#include <vector>

/**
 * Prints usage information
 */
void printUsage() {
    std::cout << "usage: bass <source-file> [output-file]\n";
}

int main(int argc, char* argv[]) {
    char* outputDirArg = nullptr;
    if (argc < 2) {
        printUsage();
        return -1;
    } else if (argc == 3) {
        outputDirArg = argv[2];
    }

    // Build data structure used to type check instruction parameters
    std::vector<InstrDefNode> instrDefs;
    buildInstrDefTree(instrDefs);

    // Create new assembler
    Assembler asmler{&instrDefs, argv[1]};

    if (!asmler.setOutputDir(outputDirArg)) {
        std::cout << "[ERROR] Output directory '" << outputDirArg
                  << "' does not exist\n";
        return -1;
    }

    bool fileReadSucc = asmler.readSource();
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
