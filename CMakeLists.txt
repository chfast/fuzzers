cmake_minimum_required(VERSION 3.22...3.27)

# Enable evmone's Hunter before the main project.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/externals/evmone/cmake/cable)
include(externals/evmone/cmake/Hunter/init.cmake)

project(fuzzers)

set(BUILD_SHARED_LIBS FALSE)
add_compile_options(-fsanitize=fuzzer)
add_link_options(-fsanitize=fuzzer-no-link)

if(COVERAGE)
    add_compile_options(
        $<$<CXX_COMPILER_ID:GNU>:--coverage>

        # Enable Clang's Source Based Coverage.
        # https://clang.llvm.org/docs/SourceBasedCodeCoverage.html
        $<$<CXX_COMPILER_ID:Clang,AppleClang>:-fprofile-instr-generate>
        $<$<CXX_COMPILER_ID:Clang,AppleClang>:-fcoverage-mapping>
    )
    add_link_options(
        $<$<CXX_COMPILER_ID:GNU>:--coverage>
        $<$<CXX_COMPILER_ID:Clang,AppleClang>:-fprofile-instr-generate>
    )
endif()

# Build with asserts
string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

add_subdirectory(common)
add_subdirectory(externals)
add_subdirectory(fuzzers)
