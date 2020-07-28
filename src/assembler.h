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
#include "ast.h"
#include "scanner.hpp"
#include "token.h"
#include <cstdint>
#include <iostream>
#include <vector>

enum class ASTState { GLOBAL_SCOPE, FUNC_BODY, INSTR_BODY, INSTR_PARAM };

class Assembler {
  public:
    Assembler();
    bool readSource(char* pathName);
    bool assemble();

    bool buildAST();

  private:
    Source* Src = nullptr;
    Scanner* Scan = nullptr;

    uint64_t TokCursor;
    std::vector<Token> Tokens;
    std::vector<ASTNode*> AST;
    void parseRegOffset(Instruction* instr);
};
