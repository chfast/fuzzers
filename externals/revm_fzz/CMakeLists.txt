set(
    LIB_LOCATION
    ${CMAKE_CURRENT_SOURCE_DIR}/target/release/${CMAKE_STATIC_LIBRARY_PREFIX}revm_fzz${CMAKE_STATIC_LIBRARY_SUFFIX}
)

add_library(revm_fzz STATIC IMPORTED GLOBAL)
target_sources(revm_fzz INTERFACE revm_fzz.hpp)
target_include_directories(revm_fzz INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(revm_fzz PROPERTIES IMPORTED_LOCATION ${LIB_LOCATION})

# Pass rust flags for coverage instrumentation.
# We want to be compatible with upstream libfuzzer.
# Inspect clang's driver -fsanitize=fuzzer option: https://github.com/llvm/llvm-project/blob/main/clang/test/Driver/fuzzer.c
# See also: https://github.com/rust-fuzz/libfuzzer
#
# The flags are set for all dependencies so this may be not ideal.
set(
    RUSTFLAGS
    -Cpasses=sancov-module

    # https://github.com/llvm/llvm-project/blob/main/llvm/lib/Transforms/Instrumentation/SanitizerCoverage.cpp#L172
    -Cllvm-args=-sanitizer-coverage-level=4

    -Cllvm-args=-sanitizer-coverage-inline-8bit-counters
    -Cllvm-args=-sanitizer-coverage-pc-table
    -Cllvm-args=-sanitizer-coverage-trace-compares

    # Some dependencies build bins so we need to provide additional linker flags to make it work.
    -Clink-args=/usr/lib/llvm-18/lib/clang/18/lib/linux/libclang_rt.fuzzer-x86_64.a
    -Clink-args=-lstdc++
)

add_custom_command(
    MAIN_DEPENDENCY src/lib.rs
    DEPENDS Cargo.toml
    OUTPUT ${LIB_LOCATION}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} -E env RUSTFLAGS='${RUSTFLAGS}' cargo build --release
)
add_custom_target(cargo_build_revm_fzz DEPENDS ${LIB_LOCATION})
add_dependencies(revm_fzz cargo_build_revm_fzz)
