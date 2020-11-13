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

#include "assembler.hpp"
#include "generator.hpp"
#include "parser.hpp"
#include <fstream>
#include <iostream>

/**
 * Constructs a new Assembler
 * @param instrDefs Vector of instruction definitons
 */
Assembler::Assembler(std::vector<InstrDefNode>* instrDefs)
    : InstrDefs(instrDefs) {}

Assembler::~Assembler() {
    delete Src;
    delete Scan;
}

bool Assembler::readSource(char* pathName) {
    std::filesystem::path p{pathName};
    if (!std::filesystem::exists(p)) {
        std::cout << "[ERROR] Source file '" << pathName
                  << "' does not exist\n";
        return false;
    }
    File = p;

    uint32_t size = 0;
    // TODO: Add error checking
    // Get source file size
    std::ifstream stream{p};
    stream.seekg(0, std::ios::end);
    size = stream.tellg();
    stream.seekg(0, std::ios::beg);

    // Write complete file into buffer
    uint8_t* buffer = new uint8_t[size];
    stream.read((char*)buffer, size);

    Src = new SourceFile(buffer, size);

    return true;
}

/**
 * Assembles the source file into a UX file
 * @return On success returns true otherwise false
 */
bool Assembler::assemble() {
    // Initialize components
    Scan = new Scanner{Src, &Tokens};

    bool scanSucc = Scan->scanSource();
    if (!scanSucc) {
        return false;
    }

    Global glob{0, 0, 0, 0};
    std::vector<LabelDefLookup> labelDefs{};

    Parser parse{InstrDefs, Src, &Tokens, &glob, &labelDefs};
    bool astSucc = parse.buildAST();
    if (!astSucc) {
        return false;
    }

    bool typeValid = parse.typeCheck();
    if (!typeValid) {
        return false;
    }

    Generator gen{&glob, &File, &labelDefs};
    gen.genBinary();

    return true;
}
