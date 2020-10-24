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
    END_OF_FILE,
};

struct Token {
    Token(TokenType type,
          uint32_t index,
          uint32_t size,
          uint32_t lineRow,
          uint32_t lineCol,
          uint8_t id);
    /** Determines the token type */
    TokenType Type;
    /** Determines the index of the token string in the source file */
    uint32_t Index = 0;
    /** Determines the size of the token string in the source file */
    uint32_t Size = 0;
    /** Determines the line row of the token in the source file */
    uint32_t LineRow = 0;
    /** Determines the line column of the token in the source file */
    uint32_t LineCol = 0;
    /**
     * Tag is either used to "tag" the token with instruction index or register
     * id. This happens in the scanning phase
     */
    uint8_t Tag = 0;
};
