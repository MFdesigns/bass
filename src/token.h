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
#include <cstdint>

enum class TokenType {
    IDENTIFIER,
    INSTRUCTION,
    LABEL_DEF,
    TYPE_INFO,
    REGISTER_DEFINITION,
    PLUS_SIGN,
    MINUS_SIGN,
    ASTERISK,
    COMMA,
    LEFT_SQUARE_BRACKET,
    RIGHT_SQUARE_BRACKET,
    LEFT_CURLY_BRACKET,
    RIGHT_CURLY_BRACKET,
    INTEGER_NUMBER,
    FLOAT_NUMBER,
    EOL,
};

struct Token {
    Token(TokenType type,
          uint32_t index,
          uint32_t size,
          uint32_t lineRow,
          uint32_t lineCol);
    TokenType Type;
    uint32_t Index;
    uint32_t Size;
    uint32_t LineRow;
    uint32_t LineCol;
};
