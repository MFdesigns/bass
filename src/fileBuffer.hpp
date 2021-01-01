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

#include <cstdint>
#include <fstream>
#include <memory>
#include <vector>

struct BufferRange {
    uint64_t Start;
    size_t Size;
    std::unique_ptr<uint8_t[]> Buffer;
    BufferRange(uint32_t start, size_t size);
    BufferRange(BufferRange& buffRange);
    BufferRange(BufferRange&& buffRange) noexcept;
};

class OutputFileBuffer {
  public:
    static const uint32_t BUFFER_SIZE = 1024;
    void reserve(size_t size);
    void push(void* src, size_t size);
    void write(size_t index, void* src, size_t size);
    void writeToStream(std::ofstream& stream);

  private:
    uint32_t Cursor = 0;
    uint32_t Capacity = 0;
    std::vector<BufferRange> Buffers;
    void allocBuffer();
};
