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

#include <cstdint>
#include <fstream>
#include <vector>

class BufferRange {
  public:
    BufferRange(uint32_t start, uint32_t end, uint8_t* buffer);
    BufferRange(BufferRange& buffRange);
    BufferRange(BufferRange&& buffRange) noexcept;
    ~BufferRange();
    uint64_t Start;
    uint64_t End;
    uint8_t* Buffer;
};

class FileBuffer {
  public:
    FileBuffer();
    ~FileBuffer();
    void increase(uint32_t size);
    void push(uint8_t data);
    void write(uint64_t index, void* data, uint32_t size);
    void writeToStream(std::ofstream& stream);
    static const uint32_t BUFFER_SIZE = 1024;

  private:
    uint32_t Cursor = 0;
    uint32_t LastBufferEnd = 0;
    uint32_t Capacity = 0;
    std::vector<BufferRange> Buffers;
    void allocBuffer();
};
