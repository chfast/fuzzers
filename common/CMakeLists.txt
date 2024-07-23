# Common utils for fuzzers

add_library(fuzz_common INTERFACE)
add_library(fuzz::common ALIAS fuzz_common)
target_include_directories(fuzz_common INTERFACE ${PROJECT_SOURCE_DIR})
target_sources(
    fuzz_common
    INTERFACE
    hex.hpp
)

