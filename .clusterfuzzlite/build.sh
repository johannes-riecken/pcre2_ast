#!/bin/bash -eu

# build project
mkdir build
cd build
cmake .. && make

# build fuzzers
$CXX $CXXFLAGS -std=c++17 \
    $SRC/pcre2_ast/fuzzer.cc -o $OUT/fuzzer \
    $LIB_FUZZING_ENGINE $SRC/pcre2_ast/build/libpcre2_ast.a -lpcre2-8
