add_executable(bass
    main.cpp
    assembler.cpp assembler.hpp
    token.cpp token.hpp
    scanner.cpp scanner.hpp
    ast.cpp ast.hpp
    parser.cpp parser.hpp
    generator.cpp generator.hpp
    fileBuffer.cpp fileBuffer.hpp
    source.cpp source.hpp
    asm/asm.cpp asm/asm.hpp
    asm/encoding.hpp
)
