#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <iostream>

using namespace std;

#include "pcre2_ast.hpp"

int main() {
  string s = "";
  auto v = from_json(s);
  if (v != nullptr) {
      cout << (to_json(std::move(v)) == s) << std::endl;
  }
  return 0;
}

