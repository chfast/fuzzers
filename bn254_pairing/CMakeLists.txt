
add_compile_options(-fsanitize=fuzzer,undefined)

block()
    set(CURVE "ALT_BN128" CACHE STRING "Default curve")
    option(IS_LIBFF_PARENT "" OFF)
    add_subdirectory(libff SYSTEM)
    add_library(libff::ff ALIAS ff)
endblock()

add_executable(bn254-pairing-fuzz)
target_link_options(bn254-pairing-fuzz PRIVATE -fsanitize=fuzzer)
target_link_libraries(bn254-pairing-fuzz PRIVATE libff::ff)
target_sources(
    bn254-pairing-fuzz
    PRIVATE
    fuzz.cpp
    impl.hpp
    impl_libff.cpp
)
