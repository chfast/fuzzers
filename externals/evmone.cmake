# evmone
# https://github.com/ethereum/evmone

set(EVMONE_TESTING TRUE)
add_subdirectory(evmone EXCLUDE_FROM_ALL)

# Fixups.
target_include_directories(evmone INTERFACE evmone/lib)
target_include_directories(evmone_precompiles INTERFACE evmone/lib)
target_link_libraries(evmone_precompiles INTERFACE evmone::evmmax)
