cmake_minimum_required(VERSION 3.22...3.27)

project(fuzzers)

set(BUILD_SHARED_LIBS FALSE)
add_compile_options(-fsanitize=fuzzer)
add_link_options(-fsanitize=fuzzer-no-link)

# Build with asserts
string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

add_subdirectory(common)
add_subdirectory(externals)
add_subdirectory(fuzzers)
