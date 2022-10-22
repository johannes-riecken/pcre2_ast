#include <iostream>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <variant>

#include "pcre2_ast.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  string s(reinterpret_cast<char const *>(data), size);
  auto v = from_json(s);
  if (v != nullptr) {
    if (std::holds_alternative<double>(*v) &&
        std::get<double>(*v) == std::get<double>(*v) - 1.0) {
      return 0;
    }
    if (to_json(std::move(v)) != s)
      __builtin_trap();
  }

  return 0;
}
