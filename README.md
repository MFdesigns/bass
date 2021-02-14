# UVM Assembler

## Building the UVM Assembler

### Requirements
- CMake
- MSVC / GCC / Clang

1. Clone repository `git clone git@github.com:MFdesigns/bass.git`
2. CD into repository folder
   - `cd bass`
3. Build application
   - `mkdir build`
   - `cd build`
   - `cmake ..`
   - `cmake --build .`
4. Run executable
   - On Windows: `./src/Debug/bass.exe`
   - On Linux: `./src/bass`

### Optional

#### Generate encoding header
Deno is required to generate the header file which contains instruction encoding information (`encoding.hpp`). To generate the file run:
- `deno run --unstable --allow-read --allow-write ./src/asm/encodingData.js`
