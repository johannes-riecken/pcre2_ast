#include <iostream>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <variant>

#include "pcre2_ast.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  string s(reinterpret_cast<char const *>(data), size);
  auto v0 = from_json(s);
  if (v0 != nullptr) {
    if (std::holds_alternative<double>(*v0) &&
        std::get<double>(*v0) == std::get<double>(*v0) - 1.0) {
      return 0;
    }
    auto v1 = to_json(std::move(v0));
    auto v2 = from_json(v1);
    if (((v0 == nullptr) ^ (v2 == nullptr)) || *v0 != *v2) {
      __builtin_trap();
    }
  }

  return 0;
}
