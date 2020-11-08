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
#include "source.hpp"
#include "token.hpp"
#include <cstdint>
#include <string>
#include <vector>

/**
 * Scans a source file and outputs an array of tokens
 */
class Scanner {
  public:
    Scanner(SourceFile* src, std::vector<Token>* outTokens);
    bool scanSource();

  private:
    /** Non owning pointer to the source file */
    SourceFile* Src = nullptr;
    /** Non owning pointer to the output token vector */
    std::vector<Token>* Tokens = nullptr;
    /** The current index into the source file */
    uint32_t Cursor = 0;
    /** The current line row at the cursor poition */
    uint32_t CursorLineRow = 1;
    /** The current line column at the cursor poition */
    uint32_t CursorLineColumn = 1;
    void incLineRow();
    void incCursor();
    char eatChar();
    char peekChar();
    void skipChar(uint32_t count);
    void skipLine();
    bool scanWord(uint32_t& outSize);
    bool scanNumber(uint32_t& outSize, bool& isFloat);
    static TokenType identifyWord(std::string& word, uint8_t* tag);
    static bool isRegister(std::string& token, uint8_t& tag);
    static bool isTypeInfo(std::string& token, uint8_t& tag);
    static bool isInstruction(std::string& token, uint8_t& tag);
};
