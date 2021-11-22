#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <iostream>

using namespace std;

#include "pcre2_ast.h"

int main() {
  string s = "[1,2,3]";
  cout << (to_json(from_json(s)) == s);
  return 0;
}

