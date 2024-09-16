add_library(geth_fzz STATIC IMPORTED GLOBAL)
target_sources(geth_fzz INTERFACE geth_fzz.h)
target_include_directories(geth_fzz INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(geth_fzz PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/geth_fzz.a)
