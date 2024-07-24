set(
    LIB_LOCATION
    ${CMAKE_CURRENT_SOURCE_DIR}/target/release/${CMAKE_STATIC_LIBRARY_PREFIX}revm_fzz${CMAKE_STATIC_LIBRARY_SUFFIX}
)

add_library(revm_fzz STATIC IMPORTED GLOBAL)
target_sources(revm_fzz INTERFACE revm_fzz.hpp)
target_include_directories(revm_fzz INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(revm_fzz PROPERTIES IMPORTED_LOCATION ${LIB_LOCATION})

add_custom_command(
    MAIN_DEPENDENCY src/lib.rs
    OUTPUT ${LIB_LOCATION}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND cargo build --release
)
add_custom_target(cargo_build_revm_fzz DEPENDS ${LIB_LOCATION})
add_dependencies(revm_fzz cargo_build_revm_fzz)
