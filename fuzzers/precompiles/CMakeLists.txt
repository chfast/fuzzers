add_executable(precompiles-fuzz)
target_compile_features(precompiles-fuzz PRIVATE cxx_std_23)
target_link_options(precompiles-fuzz PRIVATE -fsanitize=fuzzer)
target_link_libraries(precompiles-fuzz PRIVATE evmone::precompiles evmone::statetestutils)
target_sources(
    precompiles-fuzz
    PRIVATE
    common.hpp
    common.cpp
    fuzz.cpp
    state_test_mutator.cpp
)

# FIXME:
target_include_directories(
    precompiles-fuzz
    PRIVATE
    ${CMAKE_SOURCE_DIR}/externals/evmone
)
