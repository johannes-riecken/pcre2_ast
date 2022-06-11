#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <iostream>

#include "pcre2_ast.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  string s( reinterpret_cast<char const*>(data), size ) ;
    auto v = from_json(s);
    if (v != nullptr) {
        if (to_json(std::move(v)) != s)
            __builtin_trap();
    }

    return 0;
}
