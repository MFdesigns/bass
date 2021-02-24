# BASS

BASS stands for **B**ytecode **Ass**embler and can assemble files for the Universal Virtual Machine.

## Building BASS

### Requirements
- CMake
- MSVC / GCC / Clang

1. Getting the source code
   - `git clone https://github.com/MFdesigns/bass.git`
2. Building
   - `cd bass`
   - `mkdir build`
   - `cd build`
   - `cmake .. -DCMAKE_BUILD_TYPE=<build-type>`\
   Build types:
     - Debug
     - Release
    - `cmake --build .`
3. Output directory
   - On Windows: `bass/build/src/<build-type>/bass.exe`
   - On Linux/macOS: `bass/build/src/bass`

### Optional

#### Generate encoding header
Deno is required to generate the header file which contains instruction encoding information (`encoding.hpp`). To generate the file run:
- `deno run --unstable --allow-read --allow-write ./src/asm/encodingData.js`

## Generating Documentation
This project uses Doxygen to generate documentation. To output HTML documentation execute the following command in the project folder:
 - `doxygen .\Doxyfile`
