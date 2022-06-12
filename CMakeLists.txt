cmake_minimum_required(VERSION 3.10)

# set the project name
project(pcre2_ast)

find_library(PCRE2_LIBRARY_RELEASE NAMES pcre2-8 HINTS ${INSTALLED_LIB_PATH})

# add the executable
add_executable(main main.cc pcre2_ast.cpp)

add_library(pcre2_ast STATIC pcre2_ast.cpp)

include_directories(/opt/homebrew/Cellar/pcre2/10.40/include/)

target_link_libraries(pcre2_ast PUBLIC ${PCRE2_LIBRARY_RELEASE} pthread)
target_link_libraries(main PUBLIC ${PCRE2_LIBRARY_RELEASE} pthread)

target_compile_features(pcre2_ast PUBLIC cxx_std_17)
target_compile_features(main PUBLIC cxx_std_17)

