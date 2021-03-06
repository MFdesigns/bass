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

#pragma once
#include "ast.hpp"
#include "scanner.hpp"
#include "source.hpp"
#include "token.hpp"
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <vector>

class Assembler {
  public:
    Assembler(std::vector<InstrDefNode>* instrDefs, char* inFile);
    ~Assembler();
    bool setOutputDir(char* dir);
    bool readSource();
    bool assemble();

  private:
    /** Non owning pointer to instuction definitons */
    std::vector<InstrDefNode>* InstrDefs = nullptr;
    SourceFile* Src = nullptr;
    Scanner* Scan = nullptr;
    std::vector<Token> Tokens;
    std::filesystem::path InFile;
    std::filesystem::path OutFile;
};
