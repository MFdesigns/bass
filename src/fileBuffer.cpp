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

#include "fileBuffer.hpp"
#include <cstring>

BufferRange::BufferRange(uint32_t start, uint32_t end, uint8_t* buffer)
    : Start(start), End(end), Buffer(buffer) {}

BufferRange::BufferRange(BufferRange& buffRange) {
    this->Start = buffRange.Start;
    this->End = buffRange.End;
    Buffer = new uint8_t[FileBuffer::BUFFER_SIZE]();
    std::memcpy(Buffer, buffRange.Buffer, FileBuffer::BUFFER_SIZE);
}

BufferRange::BufferRange(BufferRange&& buffRange) noexcept {
    this->Start = buffRange.Start;
    this->End = buffRange.End;
    this->Buffer = buffRange.Buffer;
    buffRange.Buffer = nullptr;
}

BufferRange::~BufferRange() {
    delete[] Buffer;
}

FileBuffer::FileBuffer() {
    allocBuffer();
}

FileBuffer::~FileBuffer() {}

void FileBuffer::increase(uint32_t size) {
    // TODO: What if the size is greater then the new buffer ?
    Cursor += size;
    if (Cursor > Capacity) {
        allocBuffer();
    }
}

// ONLY WORKS WITH DATA WHICH IS <= 1024 BYTES
void FileBuffer::write(uint64_t index, void* data, uint32_t size) {
    // Find buffer with start index
    BufferRange* range = nullptr;
    bool foundBuff = false;
    auto i = 0;
    while (i < Buffers.size() && !foundBuff) {
        range = &Buffers[i];
        if (index >= range->Start && index <= range->End) {
            foundBuff = true;
        }
        i++;
    }

    // TODO: What if no range is found ?

    // If data range to be copied is inside single buffer copy data
    // Convert vAddr to relative offset of buffer array
    uint64_t offset = index - range->Start;
    if (index + size <= range->End) {
        std::memcpy(&range->Buffer[offset], data, size);
    } else {
        // TODO: This assumes that the data size is less then 1024 and that the
        // user of this class has previously allocated enough memory that an
        // adjacent buffer exists

        // If data range overlaps with two buffers perform two memcopys
        uint64_t sliceOffset = range->End - index;
        std::memcpy(&range->Buffer[offset], data, sliceOffset);
        // Get adjacent buffer and copy second data slice
        BufferRange* adjRange = &Buffers[i];
        uint64_t adjOffset = adjRange->Start - range->End;
        std::memcpy(&adjRange->Buffer[adjOffset], &static_cast<uint8_t*>(data)[sliceOffset],
                    range->End - index + 1);
    }
}

void FileBuffer::push(uint8_t data) {
    // TODO: Replace this with a member which points to the buffer which the
    // cursor points to
    BufferRange* range = nullptr;
    bool foundBuff = false;
    auto i = 0;
    while (i < Buffers.size() && !foundBuff) {
        range = &Buffers[i];
        if (Cursor >= range->Start && Cursor <= range->End) {
            foundBuff = true;
        }
        i++;
    }
    range->Buffer[Cursor] = data;
    increase(1);
}

void FileBuffer::writeToStream(std::ofstream& stream) {
    // TODO: Only write out used memory instead of all the complete buffers
    for (auto& range : Buffers) {
        stream.write((const char*)range.Buffer, FileBuffer::BUFFER_SIZE);
    }
}

void FileBuffer::allocBuffer() {
    uint8_t* newBuffer = new uint8_t[BUFFER_SIZE]();
    Buffers.push_back(
        BufferRange{LastBufferEnd, LastBufferEnd + BUFFER_SIZE, newBuffer});
    LastBufferEnd += BUFFER_SIZE;
    Capacity += BUFFER_SIZE;
}
