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

#include "../cli.hpp"
#include <iomanip>
#include <iostream>

/**
 * Prints an error to the console with colors [linux and macOS only]
 * @param Pointer to the source file where the source of the error is
 * @param index Index of the error in the source file
 * @param size Size of the error substring
 * @param row Row on which the error occured
 * @param column Column on which the error occured
 * @param msg Pointer to the error message title
 */
void printError(SourceFile* src,
                uint32_t index,
                uint32_t size,
                uint32_t row,
                uint32_t column,
                const char* msg) {

    // Left margin of the source code snippet
    constexpr uint32_t TAB = 4;

    // Get the whole line where the error occured
    std::string sourceLine;
    uint32_t lineIndex = 0;
    src->getLine(index, sourceLine, lineIndex);

    // Offset of the ~~~~~~ line
    uint32_t errOffset = index - lineIndex;

    // Char where the error occured
    uint8_t c = sourceLine[errOffset + size];

    // Get the size of the line number string
    std::string rowString = std::to_string(row);
    uint32_t rowStringMargin = rowString.size() + 3;

    // \033 ascii escape, 0 = reset colors
    constexpr char FG_WHITE[] = "\033[0m";
    // \033 ascii escape, 1 = bold, 31 = foreground red
    constexpr char FG_RED[] = "\033[1;31m";

    // Set font color to white and print error title (1. line)
    std::cerr << FG_RED << "[Error] " << msg << " (" << row << ',' << column
              << ") at char '" << c << "' (U+" << (uint16_t)c << ")" << FG_WHITE
              << "\n";

    // Set font color to error. Print line number, source code snippet and line
    // number column of 3. row
    std::cerr << "  " << row << " |" << std::setfill(' ') << std::setw(TAB)
              << ' ' << sourceLine << '\n'
              << std::setw(rowStringMargin) << std::setfill(' ') << ' ' << "|"
              << std::setfill(' ') << std::setw(TAB) << ' ';

    // If error offset (width of whitespace before the ~~~~ line begins) is zero
    // then don't output any whitespace because the way that the setw method
    // works is it always outputs the leading char atleast once resulting in one
    // space leading the ~~~~ line
    //
    // Set font color to white and print the error indicator ~~~~~~
    if (errOffset != 0) {
        std::cerr << FG_RED << std::setfill(' ') << std::setw(errOffset) << ' ';
    }

    std::cerr << std::setfill('~') << std::setw(size) << '~' << "\n\n"
              << FG_WHITE;
}
