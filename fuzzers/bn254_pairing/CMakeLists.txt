add_executable(bn254-pairing)
target_compile_features(bn254-pairing PRIVATE cxx_std_23)
target_link_options(bn254-pairing PRIVATE -fsanitize=fuzzer)
target_link_libraries(bn254-pairing PRIVATE libff::ff evmone::precompiles)
target_sources(
    bn254-pairing
    PRIVATE
    fuzz.cpp
    mutator.cpp
    impl.hpp
    impl_evmmax.cpp
    impl_libff.cpp
)

add_executable(bn254-pairing-export)
target_compile_features(bn254-pairing-export PRIVATE cxx_std_23)
target_link_libraries(bn254-pairing-export PRIVATE fuzz::common libff::ff)
target_sources(bn254-pairing-export PRIVATE export.cpp impl_libff.cpp)
