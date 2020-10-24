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

#include "token.hpp"

/**
 * Constructs a new Token
 * @param type
 * @param index Token string index in source file
 * @param size Token string size
 * @param lineRow Token string line row in source file
 * @param lineCol Token string line column in source file
 * @param tag Contains information which is passed to the parser
 */
Token::Token(TokenType type,
             uint32_t index,
             uint32_t size,
             uint32_t lineRow,
             uint32_t lineCol,
             uint8_t tag)
    : Type(type), Index(index), Size(size), LineRow(lineRow), LineCol(lineCol),
      Tag(tag){};
