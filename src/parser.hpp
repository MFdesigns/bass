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

constexpr uint8_t SEC_PERM_READ = 0b1000'0000;
constexpr uint8_t SEC_PERM_WRITE = 0b0100'0000;
constexpr uint8_t SEC_PERM_EXECUTE = 0b0010'0000;

enum class ParseState { GLOBAL_SCOPE, INSTR_BODY, END };

/**
 * This keeps track of variable position in memory so that the generator can
 * resolve references to variables in the code section
 */
struct VarDeclaration {
    /** Virtual address to variable in memory */
    uint64_t VAddr = 0;
    /** The variable name which is at this position */
    Identifier* Id = nullptr;
    /** Parent sections permission */
    uint8_t SecPerm = 0;
};

enum class RegisterType {
    INTEGER,
    FLOAT,
};

bool strToInt(std::string& str, uint64_t& num);
bool strToFP(std::string& str, double& num);

class Parser {
  public:
    Parser(std::vector<InstrDefNode>* instrDefs,
           SourceFile* src,
           const std::vector<Token>* tokens,
           ASTFileNode* fileNode,
           std::vector<LabelDefLookup>* funcDefs,
           std::vector<VarDeclaration>* varDecls);
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
    std::vector<VarDeclaration>* VarDecls;
    /** Non owning pointer to file node node */
    ASTFileNode* FileNode;
    /** Non owning pointer to source file */
    SourceFile* Src;
    Token* eatToken();
    Token* peekToken();
    void skipLine();
    void printTokenError(const char* msg, Token& tok);
    void parseString(std::string& inStr, std::string& outStr);
    bool parseRegOffset(Instruction* instr);
    bool parseSectionVars(ASTSection* sec);
    bool parseSectionCode();
    bool typeCheckInstrParams(Instruction* instr,
                              std::vector<Identifier*>& labelRefs);
    bool typeCheckVars(ASTSection* sec);
    bool checkVarRefs();
};
