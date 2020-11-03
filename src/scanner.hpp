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
#include <array>
#include <cstdint>
#include <string>
#include <vector>

class Scanner {
  public:
    Scanner(SourceFile* src, std::vector<Token>* outTokens);
    bool scanSource();

  private:
    uint32_t Cursor = 0;
    uint32_t CursorLineRow = 1;
    uint32_t CursorLineColumn = 1;
    SourceFile* Src = nullptr;
    std::vector<Token>* Tokens = nullptr;
    void throwError(const char* msg, uint32_t start);
    void skipLine();
    bool scanWord(uint32_t& outSize);
    bool isRegister(std::string& token, uint8_t& tag);
    bool isTypeInfo(std::string& token, uint8_t& tag);
    bool isInstruction(std::string& token, uint8_t& tag);
    void addToken(TokenType type,
                  uint32_t index,
                  uint32_t lineRow,
                  uint32_t lineColumn,
                  uint32_t size,
                  uint8_t tag);
    bool peekChar(uint8_t& out);
    bool eatChars(uint32_t count, uint8_t& out);
    bool eatChar(uint8_t& out);
    void incLineRow();
    void incCursor();
};
