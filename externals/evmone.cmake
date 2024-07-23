# evmone
# https://github.com/ethereum/evmone
add_subdirectory(evmone EXCLUDE_FROM_ALL)

# Fixups.
target_include_directories(evmone_precompiles INTERFACE evmone/lib)
target_link_libraries(evmone_precompiles INTERFACE evmone::evmmax)
