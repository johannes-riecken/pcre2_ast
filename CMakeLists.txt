cmake_minimum_required(VERSION 3.10)

# set the project name
project(pcre2_ast)

find_library(PCRE2_LIBRARY_RELEASE NAMES pcre2-8 HINTS ${INSTALLED_LIB_PATH})

find_library(GTEST_LIBRARY gtest)

## This fails with CodeQL build
# add_compile_options(-fsanitize=address -fsanitize=undefined)
# add_link_options(-fsanitize=address -fsanitize=undefined)

enable_testing()
add_executable(pcre2_ast_test pcre2_ast_test.cpp)
include(GoogleTest)
gtest_discover_tests(pcre2_ast_test)
target_link_libraries(pcre2_ast_test PUBLIC ${PCRE2_LIBRARY_RELEASE} ${GTEST_LIBRARY} pcre2_ast pthread c++)
target_compile_features(pcre2_ast_test PUBLIC cxx_std_17)

# add the executable
add_executable(main main.cc)

add_library(pcre2_ast pcre2_ast.cpp)

include_directories(/opt/homebrew/Cellar/pcre2/10.40/include/)
include_directories(/usr/local/Cellar/pcre2/10.40/include/)
include_directories(/usr/local/Cellar/googletest/1.12.1_1/include/)

target_link_libraries(pcre2_ast PUBLIC ${PCRE2_LIBRARY_RELEASE} pthread c++)
target_link_libraries(main PUBLIC ${PCRE2_LIBRARY_RELEASE} pthread c++ pcre2_ast)

# make sure the executables can access the files in res/
add_custom_command(TARGET pcre2_ast_test PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/res/ $<TARGET_FILE_DIR:pcre2_ast_test>)

target_compile_features(pcre2_ast PUBLIC cxx_std_17)
target_compile_features(main PUBLIC cxx_std_17)

