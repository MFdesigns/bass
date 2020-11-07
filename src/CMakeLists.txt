set(PROJECT_NAME bass)

set(SOURCE_FILES
    main.cpp
    assembler.cpp assembler.hpp
    token.cpp token.hpp
    scanner.cpp scanner.hpp
    ast.cpp ast.hpp
    parser.cpp parser.hpp
    generator.cpp generator.hpp
    fileBuffer.cpp fileBuffer.hpp
    source.cpp source.hpp
    cli.hpp
    asm/asm.cpp asm/asm.hpp
    asm/encoding.hpp
)

# Win32 specific platform files
if(WIN32)
    set(PLATFORM_FILES
        platform/win32Platform.cpp
    )
# Linux and MacOS shared platform files
elseif(UNIX)
    set(PLATFORM_FILES
        platform/unixPlatform.cpp
    )
    # MacOS specific platform files
    if(APPLE)
        # Append additional macos files to unix
    endif()
endif()

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${PLATFORM_FILES})
