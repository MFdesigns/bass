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
#include <memory>
#include <string>

/**
 * Represents a source file
 */
class SourceFile {
  public:
    SourceFile(uint8_t* data, uint32_t size);
    uint32_t getSize();
    // TODO: Deprecate this
    uint8_t* getData();
    bool getChar(uint32_t index, char& c);
    bool getSubStr(uint32_t index, uint32_t size, std::string& out);
    bool getLine(uint32_t index, std::string& out, uint32_t& lineIndex);

  private:
    /** Raw file buffer */
    std::unique_ptr<uint8_t> Data;
    /** File buffer size */
    const uint32_t Size = 0;
};
