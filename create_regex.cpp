#include <iostream>
#include <sstream>

enum command {     // comments are factor expressions
  create_array,    // { }
  push_back_array, // suffix
  push_number,     // 42
  push_string,     // "string"
  create_map,      //  H{ }
  push_back_map,   // ?set-at value key assoc/f
  push_true,       // t
  push_false,      // f
  push_null,       // null
};

int main() {
  std::stringstream ss;
  // to simplify trouble-shooting, we'll test the sub-expressions first
  ss << "(*NO_AUTO_POSSESS)(*NO_DOTSTAR_ANCHOR)(*NO_START_OPT)\n"
        "\\A (?&json_val) \\z\n"
        "(?(DEFINE)\n"
        "(?<json_val>\n"
        "\\s*\n"
        "(\n"
        "    (?&string)\n"
        "  |\n"
        "    (?&push_number)\n"
        "  |\n"
        "    (?&object)\n"
        "  |\n"
        "    (?&array)\n"
        "  |\n"
        "  true (?C"
     << push_true
     << ")\n"
        "|\n"
        "  false (?C"
     << push_false
     << ")\n"
        "|\n"
        "  null (?C"
     << push_null
     << ")\n"
        ")\n"
        "\\s*\n"
        ")\n"
        "(?<string> \n"
        "  (\n"
        "    \"\n"
        "    (?:\n"
        "      [^\\\\\"]+\n"
        "    |\n"
        "      \\\\ [\"\\\\/bfnrt]\n"
        "    )*\n"
        "    \"\n"
        "  )\n"
        "(?C"
     << push_string
     << ")\n"
        ")\n"
        "(?<object> \\{ (?C"
     << create_map << ") ( (?&key) : (?&value) (?C" << push_back_map
     << ")\n"
        "( , \\s* (?&key) : (?&value) (?C"
     << push_back_map
     << "))* )? \\} )\n"
        "(?<key> (?&string) )\n"
        "(?<value> (?&json_val) )\n"
        "(?<push_number>\n"
        "  (\n"
        "    -?\n"
        "    (?: 0 | [1-9]\\d* )\n"
        "    (?: \\. \\d+ )?\n"
        "    (?: [eE] [-+]? \\d+ )?\n"
        "  )\n"
        "  (?C"
     << push_number
     << ")\n"
        ")\n"
        "(?<array> \\[ (?C"
     << create_array << ") (?: (?&json_val) (?C" << push_back_array
     << ")\n"
        "(?: , (?&json_val) (?C"
     << push_back_array
     << "))* )? \\] )\n"
        ")";
  std::cout << ss.str() << std::endl;
  return 0;
}
