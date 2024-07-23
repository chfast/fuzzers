# libff
# https://github.com/scipr-lab/libff
# Fork for fuzzing: chfast/libff, branch "fuzzing".

set(CURVE "ALT_BN128" CACHE STRING "Default curve")
option(IS_LIBFF_PARENT "" OFF)
add_subdirectory(libff SYSTEM)
add_library(libff::ff ALIAS ff)
