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
#include "asm/asm.hpp"
#include "ast.hpp"
#include "scanner.hpp"
#include "token.hpp"
#include <vector>

enum class ParseState { GLOBAL_SCOPE, INSTR_BODY, END };

class Parser {
  public:
    Parser(std::vector<InstrDefNode>* instrDefs,
           SourceFile* src,
           const std::vector<Token>* tokens,
           Global* global,
           std::vector<LabelDefLookup>* funcDefs);
    bool buildAST();
    bool typeCheck();

  private:
    uint64_t Cursor = 0;

    /** Non owning pointer to instruction definitons */
    std::vector<InstrDefNode>* InstrDefs = nullptr;
    /** Non owning vector of token */
    const std::vector<Token>* Tokens = nullptr;
    /** Vector of non owning pointers to function declarations */
    std::vector<LabelDefLookup>* LabelDefs = nullptr;
    /** Non owning pointer to global AST node */
    Global* Glob;
    SourceFile* Src;
    int64_t strToInt(std::string& str);
    Token* eatToken();
    Token* peekToken();
    void printTokenError(const char* msg, Token& tok);
    bool parseRegOffset(Instruction* instr);
    bool typeCheckInstrParams(Instruction* instr,
                              std::vector<Identifier*>& labelRefs);
};
