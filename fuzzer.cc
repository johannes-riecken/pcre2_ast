#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <iostream>

#include "pcre2_ast.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  string s = "[1, 2, 3]";
  if (to_json(from_json(s)) != s)
    __builtin_trap();
  return 0;
}

extern "C" int FuzzOneInput(const uint8_t *data, size_t size) {
  string s( reinterpret_cast<char const*>(data), size ) ;
  if (to_json(from_json(s)) != s)
    __builtin_trap();
  return 0;
}
