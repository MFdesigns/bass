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

#include "fileBuffer.hpp"
#include <cstring>

/**
 * Constructs a new BufferRange
 * @param Index of buffer
 * @param Size of buffer
 */
BufferRange::BufferRange(uint32_t start, size_t size)
    : Start(start), Size(size),
      Buffer(std::make_unique<uint8_t[]>(OutputFileBuffer::BUFFER_SIZE)) {}

/**
 * Copy constructs a new BufferRange
 */
BufferRange::BufferRange(BufferRange& buffRange) {
    this->Start = buffRange.Start;
    this->Size = buffRange.Size;
    this->Buffer = std::move(buffRange.Buffer);
}

/**
 * Move constructs a new BufferRange
 */
BufferRange::BufferRange(BufferRange&& buffRange) noexcept {
    this->Start = buffRange.Start;
    this->Size = buffRange.Size;
    this->Buffer = std::move(buffRange.Buffer);
}

/**
 * Reserves buffer of size, allocates new buffer range if necessary and
 * increases the internal cursor
 * @param Size to be reserved
 */
void OutputFileBuffer::reserve(size_t size) {
    size_t neededCapacity = Cursor + size;
    if (neededCapacity > Capacity) {
        allocBuffer();
    }
    Cursor += size;
}

/**
 * Writes to already reserved memory. Warning: This only supports blocks up to
 * BUFFER_SIZE
 * @param index Destination index
 * @param src Pointer to source
 * @param size Size of memory to be copied
 */
void OutputFileBuffer::write(size_t index, void* src, size_t size) {
    // Find buffer with start index
    BufferRange* range = nullptr;
    size_t rangeIndex = 0;
    for (const BufferRange& buff : Buffers) {
        if (index >= buff.Start && index < buff.Start + buff.Size) {
            range = const_cast<BufferRange*>(&buff);
            break;
        }
        rangeIndex++;
    }

    // If data range to be copied is inside single buffer copy data
    // Convert vAddr to relative offset of buffer array
    uint64_t offset = index - range->Start;
    size_t buffEnd = range->Start + range->Size;
    if (index + size <= buffEnd) {
        std::memcpy(&range->Buffer[offset], src, size);
    } else {
        // If data range overlaps with two buffers perform two memcopys
        uint64_t sliceOffset = buffEnd - index;
        std::memcpy(&range->Buffer[offset], src, sliceOffset);

        // Get adjacent buffer and copy second data slice
        BufferRange* adjRange = nullptr;
        if (Buffers.size() > rangeIndex + 1) {
            adjRange = &Buffers[rangeIndex + 1];
        }

        uint64_t adjOffset = adjRange->Start - buffEnd;
        std::memcpy(&adjRange->Buffer[adjOffset],
                    &static_cast<uint8_t*>(src)[sliceOffset],
                    buffEnd - index + 1);
    }
}

/**
 * Copy data at the end of the buffer indicated by the internal Cursor
 * @param src Pointer to source
 * @param size Size of memory to be copied
 */
void OutputFileBuffer::push(void* src, size_t size) {
    size_t neededCapacity = Cursor + size;
    if (neededCapacity > Capacity) {
        allocBuffer();
    }

    write(Cursor, src, size);
    Cursor += size;
}

/**
 * Writes the buffer content to an output file stream
 * @param stream Reference to stream
 */
void OutputFileBuffer::writeToStream(std::ofstream& stream) {
    for (auto& range : Buffers) {
        size_t writeSize = OutputFileBuffer::BUFFER_SIZE;
        if (Cursor < range.Start + range.Size) {
            writeSize = Cursor - range.Start;
        }
        stream.write((const char*)(range.Buffer.get()), writeSize);
    }
}

/**
 * Allocates a new buffer and increases the Cursor by the BUFFER_SIZE
 */
void OutputFileBuffer::allocBuffer() {
    Buffers.emplace_back(Capacity, BUFFER_SIZE);
    Capacity += BUFFER_SIZE;
}
