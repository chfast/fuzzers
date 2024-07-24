add_library(revm_fzz STATIC IMPORTED GLOBAL)
target_sources(revm_fzz INTERFACE revm_fzz.hpp)
target_include_directories(revm_fzz INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(revm_fzz PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/target/release/librevm_fzz.a)
