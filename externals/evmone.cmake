# evmone
# https://github.com/ethereum/evmone

set(EVMONE_TESTING ON CACHE BOOL "Build tests and test tools" FORCE)
set(EVMONE_PRECOMPILES_SILKPRE ON CACHE BOOL "" FORCE)
add_compile_options(-fsanitize=fuzzer-no-link)
add_subdirectory(evmone EXCLUDE_FROM_ALL)

# Fixups.
target_include_directories(evmone_precompiles INTERFACE evmone/lib)
target_link_libraries(evmone_precompiles INTERFACE evmone::evmmax)
