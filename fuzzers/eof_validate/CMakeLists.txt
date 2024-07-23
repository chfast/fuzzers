add_executable(eof-validate)
target_link_libraries(eof-validate PRIVATE evmone evmone::testutils)
target_sources(
    eof-validate
    PRIVATE
    fuzz.cpp
)
