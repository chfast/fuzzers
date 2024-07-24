add_executable(eof-validate)
target_link_libraries(eof-validate PRIVATE evmone evmone::testutils revm_fzz)
target_link_options(eof-validate PRIVATE -fsanitize=fuzzer)
target_sources(
    eof-validate
    PRIVATE
    fuzz.cpp
)
