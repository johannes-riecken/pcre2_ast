#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <iostream>

#include "pcre2_ast.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  string s = "[1, 2, 3]";
  if (to_json(from_json(s)) != s)
    __builtin_trap();
  return 0;
}
