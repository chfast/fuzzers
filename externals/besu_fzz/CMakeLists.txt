find_package(JNI REQUIRED)
find_package(Java REQUIRED)

set(BESU_DIR ${PROJECT_SOURCE_DIR}/../besu)
cmake_path(ABSOLUTE_PATH BESU_DIR NORMALIZE OUTPUT_VARIABLE BESU_DIR)
file(GLOB BESU_CLASSPATH ${BESU_DIR}/build/install/besu/lib/*.jar)
string(JOIN ":" BESU_CLASSPATH ${BESU_CLASSPATH})
#message("besu classpath: ${BESU_CLASSPATH}")
file(GENERATE OUTPUT besu_classpath.cpp CONTENT "const char* besu_classpath = \"-Djava.class.path=${CMAKE_CURRENT_BINARY_DIR}:${BESU_CLASSPATH}\";")

add_library(besu_fzz)
target_link_libraries(besu_fzz PRIVATE JNI::JVM)
target_include_directories(besu_fzz PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(
    besu_fzz PRIVATE
    besu_fzz.hpp
    besu_fzz.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/besu_classpath.cpp
    BesuFzz.java
)

add_custom_command(
    TARGET besu_fzz
    PRE_BUILD
    COMMAND
    ${Java_JAVAC_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/BesuFzz.java
    -d ${CMAKE_CURRENT_BINARY_DIR}
    -classpath ${BESU_CLASSPATH}
    -proc:none
    BYPRODUCTS BesuFzz.class
)
