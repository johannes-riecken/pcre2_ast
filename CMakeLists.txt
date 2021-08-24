cmake_minimum_required(VERSION 3.10)

# set the project name
project(pcre2_ast)

find_library(PCRE2_LIBRARY_RELEASE NAMES pcre2-8 HINTS ${INSTALLED_LIB_PATH})

# add the executable
add_executable(pcre2_ast pcre2_ast.cpp)

target_link_libraries(pcre2_ast PUBLIC ${PCRE2_LIBRARY_RELEASE})

target_compile_features(pcre2_ast PUBLIC cxx_std_17)

