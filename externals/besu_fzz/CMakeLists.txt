find_package(JNI REQUIRED)

get_filename_component(BESU_DIR ${PROJECT_SOURCE_DIR}/../besu ABSOLUTE)
file(GLOB BESU_CLASSPATH ${BESU_DIR}/build/install/besu/lib/*.jar)
message("${BESU_JARS}")
#file(WRITE ${CMAKE_CURRENT_BINARY_DIR})

add_library(besu_fzz besu_fzz.hpp besu_fzz.cpp)
target_link_libraries(besu_fzz PRIVATE JNI::JVM)
target_include_directories(besu_fzz PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
