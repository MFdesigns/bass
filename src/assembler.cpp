// ======================================================================== //
// Copyright 2020-2021 Michel Fäh
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
 * @param inFile Source file path
 * @param outFile Output file path. Use default by passing nullptr
 */
Assembler::Assembler(std::vector<InstrDefNode>* instrDefs, char* inFile)
    : InstrDefs(instrDefs), InFile(inFile) {}

/**
 * Assembler destructor
 */
Assembler::~Assembler() {
    delete Src;
    delete Scan;
}

/**
 * Sets the output directory. To select default output directory pass nullptr
 * @param dir Output path or nullptr
 * @return On success return true otherwise false
 */
bool Assembler::setOutputDir(char* dir) {
    if (dir == nullptr) {
        OutFile = InFile;
        OutFile.replace_filename("a.ux");
        return true;
    }

    OutFile = dir;
    if (!std::filesystem::exists(OutFile.parent_path())) {
        return false;
    }

    return true;
}

/**
 * Reads the previously set source file into a buffer
 * @return On success return true otherwise false
 */
bool Assembler::readSource() {
    if (!std::filesystem::exists(InFile)) {
        return false;
    }

    uint32_t size = 0;
    // Get source file size
    std::ifstream stream{InFile};
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

    ASTFileNode fileNode{};
    std::vector<LabelDefLookup> labelDefs;
    std::vector<VarDeclaration> VarDecls;

    Parser parse{InstrDefs, Src, &Tokens, &fileNode, &labelDefs, &VarDecls};
    bool astSucc = parse.buildAST();
    if (!astSucc) {
        return false;
    }

    bool typeValid = parse.typeCheck();
    if (!typeValid) {
        return false;
    }

    Generator gen{&fileNode, &OutFile, &labelDefs, &VarDecls};
    gen.genBinary();

    return true;
}
