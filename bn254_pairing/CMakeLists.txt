
if(CMAKE_BUILD_TYPE STREQUAL Coverage)
    add_compile_options(-g -fprofile-instr-generate -fcoverage-mapping)
    add_link_options(-g -fprofile-instr-generate -fcoverage-mapping)
else()
    add_compile_options(-fsanitize=fuzzer,undefined,address)
endif()

block()
    set(CURVE "ALT_BN128" CACHE STRING "Default curve")
    option(IS_LIBFF_PARENT "" OFF)
    add_subdirectory(libff SYSTEM)
    add_library(libff::ff ALIAS ff)
endblock()

add_executable(bn254-pairing-fuzz)
target_compile_features(bn254-pairing-fuzz PRIVATE cxx_std_23)
target_link_options(bn254-pairing-fuzz PRIVATE -fsanitize=fuzzer,undefined,address)
target_link_libraries(bn254-pairing-fuzz PRIVATE libff::ff)
target_sources(
    bn254-pairing-fuzz
    PRIVATE
    fuzz.cpp
    mutator.cpp
    impl.hpp
    impl_libff.cpp
)
