// Inspired by https://www.perlmonks.org/?node_id=995856
#define PCRE2_CODE_UNIT_WIDTH 8
#include <array>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stack>
#include <string>
#include <variant>
#include <vector>

#include <pcre2.h>

#include "pcre2_ast.h"

constexpr bool is_debug = false;

using namespace std;


enum command {
  create_array,
  push_back_array,
  push_number,
  push_string,
  create_map,
  push_back_map,
  push_true,
  push_false,
  push_null,
};

string command_to_string(command c) {
  switch (c) {
  case create_array:
    return "create_array";
  case push_back_array:
    return "push_back_array";
  case push_number:
    return "push_number";
  case push_string:
    return "push_string";
  case create_map:
    return "create_map";
  case push_back_map:
    return "push_back_map";
  case push_true:
    return "push_true";
  case push_false:
    return "push_false";
  case push_null:
    return "push_null";
  }
}

string escape_string(const string& s) {
    stringstream ret;
    ret << "\"";
    for (char c : s) {
        switch (c) {
            case '"':
                ret << "\\\"";
                break;
            case '\\':
                ret << "\\\\";
                break;
            case '/':
                ret << "\\/";
                break;
            case '\b':
                ret << "\\b";
                break;
            case '\f':
                ret << "\\f";
                break;
            case '\n':
                ret << "\\n";
            case '\r':
                ret << "\\r";
                break;
            case '\t':
                ret << "\\t";
                break;
            default:
                ret << c;
        }
    }
    ret << "\"";
    return ret.str();
}

string parse_json_string(string s) {
    stringstream ret;
    s = s.substr(1, s.size() - 2); // remove quotes

    bool after_backslash = false;
    for (char c : s) {
        if (after_backslash) {
            switch (c) {
                case '"':
                    ret << "\"";
                    break;
                case '\\':
                    ret << "\\";
                    break;
                case '/':
                    ret << "/";
                    break;
                case 'b':
                    ret << "\b";
                    break;
                case 'f':
                    ret << "\f";
                case 'n':
                    ret << "\n";
                    break;
                case 'r':
                    ret << "\r";
                    break;
                case 't':
                    ret << "\t";
                    break;
                default:
                    throw exception {};
            }
            after_backslash = false;
        } else if (c == '\\') {
            after_backslash = true;
        } else {
            ret << c;
        }
    }
    return ret.str();
}


string to_json(unique_ptr<JsonValue> v) {
    stringstream ret;
    if (holds_alternative<map<string, unique_ptr<s>>>(*v)) {
        ret << "{";
        auto m = move(get<map<string, unique_ptr<s>>>(*v));
        auto times_run = 0;
        for (auto& kv : m) {
            if (times_run++) {
                ret << ", ";
            }
            ret << "\"" << kv.first << "\"" << ": " << to_json(make_unique<JsonValue>(move(kv.second->v)));
        }
        ret << "}";
    } else if (holds_alternative<double>(*v)) {
        auto n = get<double>(*v);
        ret << n;
    } else if (holds_alternative<string>(*v)) {
        auto str = move(get<string>(*v));
        ret << escape_string(str);
    } else if (holds_alternative<vector<unique_ptr<s>>>(*v)) {
        ret << "[";
        auto vec = move(get<vector<unique_ptr<s>>>(*v));
        auto times_run = 0;
        for (auto& e : vec) {
            if (times_run++) {
                ret << ", ";
            }
            ret << to_json(make_unique<JsonValue>(move(e->v)));
        }
        ret << "]";
    } else if (holds_alternative<bool>(*v)) {
      auto b = get<bool>(*v);
      ret << (b ? "true" : "false");
    } else if (holds_alternative<monostate>(*v)) {
      ret << "null";
    }
    return ret.str();
}

auto st = deque<unique_ptr<JsonValue>>{};

static int callout_handler(pcre2_callout_block *c, void *data) {
  if (is_debug) {
    cout << command_to_string(static_cast<command>(c->callout_number)) << endl;
  }
  switch (c->callout_number) {
  case create_array: {
    JsonValue val = vector<unique_ptr<s>> {};
    st.push_back(make_unique<JsonValue>(move(val)));
  } break;
  case push_back_array: {
    unique_ptr<JsonValue> x = move(st.back());
    st.pop_back();
    unique_ptr<JsonValue> vec_variant = move(st.back());
    st.pop_back();
    vector<unique_ptr<s>> vec = move(get<vector<unique_ptr<s>>>(*vec_variant));
    vec.push_back(unique_ptr<s>(new s { .v = move(*x) }));
    st.push_back(make_unique<JsonValue>(move(vec)));
  } break;
  case push_number: {
    auto begin_offset = c->offset_vector[c->capture_last * 2];
    auto end_offset   = c->offset_vector[c->capture_last * 2 + 1];
    string subject { (char*)c->subject };
    auto val_str = subject.substr(begin_offset, end_offset - begin_offset);
    st.push_back(make_unique<JsonValue>(stod(val_str)));
  } break;
  case push_string: {
    auto begin_offset = c->offset_vector[c->capture_last * 2];
    auto end_offset   = c->offset_vector[c->capture_last * 2 + 1];
    string subject { (char*)c->subject };
    JsonValue val_str = parse_json_string(subject.substr(begin_offset, end_offset - begin_offset));
    st.push_back(make_unique<JsonValue>(move(val_str)));
  } break;
  case create_map: {
    st.push_back(make_unique<JsonValue>(map<string, unique_ptr<s>> {}));
  } break;
  case push_back_map: {
    unique_ptr<JsonValue> v = move(st.back());
    st.pop_back();
    string k = get<string>(*move(st.back()));
    st.pop_back();
    map<string, unique_ptr<s>> m = move(get<map<string, unique_ptr<s>>>(*st.back()));
    st.pop_back();
    m[k] = unique_ptr<s> { new s { .v = move(*v) } };
    st.push_back(make_unique<JsonValue>(move(m)));
  } break;
  case push_true: {
      st.push_back(move(make_unique<JsonValue>(true)));
  } break;
  case push_false: {
    st.push_back(move(make_unique<JsonValue>(false)));
  } break;
  case push_null: {
    st.push_back(move(make_unique<JsonValue>(monostate {})));
  } break;
  default: {
    cout << "Exception is exceptional" << endl;
    throw exception{};
  }
  }
  return 0;
}


unique_ptr<JsonValue> from_json(const string& str) {
  pcre2_match_context *match_context = pcre2_match_context_create(nullptr);
  pcre2_set_callout(match_context, callout_handler, nullptr);
  stringstream ss;
  ss << "(*NO_AUTO_POSSESS)(*NO_DOTSTAR_ANCHOR)(*NO_START_OPT)"
        "\\A (?&json_val) \\z"
        "(?(DEFINE)"
        "(?<json_val>"
        "\\s*"
        "("
        "    (?&string)"
        "  |"
        "    (?&push_number)"
        "  |"
        "    (?&object)"
        "  |"
        "    (?&array)"
        "  |"
        "  true (?C" << push_true << ")"
        "|"
        "  false (?C" << push_false << ")"
        "|"
        "  null (?C" << push_null << ")"
        ")"
        "\\s*"
        ")"
        "(?<string> "
        "  ("
        "    \""
        "    (?:"
        "      [^\\\\\"]+"
        "    |"
        "      \\ [\"\\\\/bfnrt]"
        "    )*"
        "    \""
        "  )"
        "(?C" << push_string << ")"
        ")"
        "(?<object> \\{ (?C" << create_map << ") ( (?&key) : (?&value) (?C" << push_back_map << ")"
        "( , \\s* (?&key) : (?&value) (?C" << push_back_map << "))* )? \\} )"
        "(?<key> (?&string) )"
        "(?<value> (?&json_val) )"
        "(?<push_number>"
        "  ("
        "    -?"
        "    (?: 0 | [1-9]\\d* )"
        "    (?: \\. \\d+ )?"
        "    (?: [eE] [-+]? \\d+ )?"
        "  )"
        "  (?C" << push_number << ")"
        ")"
        "(?<array> \\[ (?C" << create_array << ") (?: (?&json_val) (?C" << push_back_array << ")"
        "(?: , (?&json_val) (?C" << push_back_array << "))* )? \\] )"

        ")";
  // most of the following code is copied from libpcre2's pcre2demo.c
  auto ss_str = ss.str();
  auto pattern = reinterpret_cast<PCRE2_SPTR>(ss_str.c_str());
  auto subject = reinterpret_cast<PCRE2_SPTR>(str.c_str());
  auto subject_length = static_cast<PCRE2_SIZE>(strlen(reinterpret_cast<const char *>(subject)));

  int errornumber;
  PCRE2_SIZE erroroffset;
  pcre2_code *re = pcre2_compile(
      pattern,                      /* the pattern */
      PCRE2_ZERO_TERMINATED,        /* indicates pattern is zero-terminated */
      PCRE2_EXTENDED, &errornumber, /* for error push_number */
      &erroroffset,                 /* for error offset */
      nullptr);                     /* use default compile context */

  if (re == nullptr) {
    constexpr int BUF_SZ = 256;
    array<PCRE2_UCHAR, BUF_SZ> buffer {};
    pcre2_get_error_message(errornumber, buffer.data(), buffer.size());
    cout << "PCRE2 compilation failed at offset " << erroroffset << ": "
         << buffer.data() << endl;
    return nullptr;
  }

  pcre2_match_data *match_data =
      pcre2_match_data_create_from_pattern(re, nullptr);

  int rc = pcre2_match(re,             /* the compiled pattern */
                       subject,        /* the subject string */
                       subject_length, /* the length of the subject */
                       0,              /* start at offset 0 in the subject */
                       0,              /* default options */
                       match_data,     /* block for storing the result */
                       match_context); /* use default match context */
  /* nullptr);       /1* use default match context *1/ */

  /* Matching failed: handle error cases */

  if (rc < 0) {
    if (rc == PCRE2_ERROR_NOMATCH) {
        cout << "No match" << endl;
    } else {
      cout << "Matching error: " << rc << endl;
    }
    pcre2_match_data_free(match_data); /* Release memory used for the match */
    pcre2_code_free(re);               /*   data and the compiled pattern. */
    pcre2_match_context_free(match_context);
    return nullptr;
  }

  /* Match succeded. Get a pointer to the output vector, where string offsets
  are stored. */

  PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);

  /*************************************************************************
   * We have found the first match within the subject string. If the output *
   * vector wasn't big enough, say so. Then output any substrings that were *
   * captured.                                                              *
   *************************************************************************/

  /* The output vector wasn't big enough. This should not happen, because we
  used pcre2_match_data_create_from_pattern() above. */

  if (rc == 0) {
      cout << "ovector was not big enough for all the captured substrings" << endl;
  }

  /* We must guard against patterns such as /(?=.\K)/ that use \K in an
  assertion to set the start of a match later than its end. In this
  demonstration program, we just detect this case and give up. */

  if (ovector[0] > ovector[1]) {
    cout <<
        "\\K was used in an assertion to set the match start after its end.\n";
        /* "From end to start the match was: %.*s\n", */
        /* (int)(ovector[0] - ovector[1]), (char *)(subject + ovector[1]); */
    cout << "Run abandoned" << endl;
    pcre2_match_data_free(match_data);
    pcre2_code_free(re);
    pcre2_match_context_free(match_context);
    return nullptr;
  }
  pcre2_match_data_free(match_data);
  pcre2_code_free(re);
  pcre2_match_context_free(match_context);
  return move(st.back());
}
// int main() {
//   cout << to_json(from_json("[1,2,3]")) << endl;
//   cout << to_json(from_json(R"({"a":[1,2,3.14], "b":null, "c":{"d":[true,false]}})")) << endl;
//   return 0;
// }
