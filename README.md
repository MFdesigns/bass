# UVM Assembler

## Building the UVM Assembler

### Requirements
- Deno
- CMake
- MSVC / GCC / Clang

1. Clone repository `git clone git@github.com:MFdesigns/universal-assembler.git`
2. Generate header files
   - `cd universal-assembler`
   - `deno run --unstable --allow-read --allow-write ./src/asm/encodingData.js`
3. Build application
   - `mkdir build`
   - `cd build`
   - `cmake ..`
   - `cmake --build .`
4. Run executable
   - On Windows: `./src/Debug/bass.exe`
   - On Linux: `./src/bass`
