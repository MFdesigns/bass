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

#include "source.hpp"

/**
 * Constructs a new SourceFile
 * @param data Pointer to file buffer
 * @param size Size of file buffer
 */
SourceFile::SourceFile(uint8_t* data, uint32_t size) : Data(data), Size(size) {}

/**
 * Gets the size of file buffer
 * @return File buffer size
 */
uint32_t SourceFile::getSize() {
    return Size;
}

/**
 * Gets the raw file buffer
 * @return Pointer to file buffer
 */
uint8_t* SourceFile::getData() {
    return Data.get();
}

/**
 * Returns char at given position
 * @param index Index into file buffer
 * @param c [out] Character at given index
 * @return If index is in range of file buffer returns true otherwise false
 */
bool SourceFile::getChar(uint32_t index, char& c) {
    if (index < Size) {
        c = Data.get()[index];
        return true;
    } else {
        return false;
    }
}

/**
 * Gets a substring of the file buffer
 * @param index Index into file buffer
 * @param size Size of substring
 * @param out [out] String which will be filled with the substring
 * @return On success returns true otherwise false
 */
bool SourceFile::getSubStr(uint32_t index, uint32_t size, std::string& out) {
    if (index < Size && index + size < Size) {
        out.clear();
        out.reserve(size);
        char c;
        for (auto i = 0; i < size; i++) {
            getChar(index + i, c);
            out.push_back(c);
        }
        return true;
    } else {
        return false;
    }
}

/**
 * Gets a complete source file line of the given index
 * @param index Index into file buffer
 * @param out [out] String which will be filled with the line
 * @param lineIndex [out] Contains the line number
 * @return On success returns true otherwise false
 */
bool SourceFile::getLine(uint32_t index,
                         std::string& out,
                         uint32_t& lineIndex) {
    if (index >= Size) {
        return false;
    }

    // Non owning pointer
    uint8_t* DataPtr = Data.get();

    // Find the begining of the line
    bool foundSOL = false;
    while (!foundSOL && (int32_t)index - 1 > 0) {
        if (DataPtr[index - 1] == '\n') {
            foundSOL = true;
        } else {
            index--;
        }
    }
    lineIndex = index;

    // Parse the whole line
    uint8_t c = DataPtr[index];
    while (c != '\n' && index < Size) {
        index++;
        out.push_back(c);
        c = DataPtr[index];
    }

    return true;
}
