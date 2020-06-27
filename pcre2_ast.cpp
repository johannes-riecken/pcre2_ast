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

using namespace std;

struct s;
using Value = variant<map<string, unique_ptr<s>>, int, string, vector<unique_ptr<s>>>;
struct s {
    Value v;
    ~s();
};
inline s::~s() = default;

string escape_string(string s) {
    return s;
}

string parse_json_string(string s) {
    stringstream ret;
    s = s.substr(1, s.size() - 2); // remove quotes

    bool after_backslash = false;
    for (char& c : s) {
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


string to_json(unique_ptr<Value> v) {
    stringstream ret;
    if (holds_alternative<map<string, unique_ptr<s>>>(*v)) {
        ret << "{";
        auto m = move(get<map<string, unique_ptr<s>>>(*v));
        auto times_run = 0;
        for (auto& kv : m) {
            if (times_run++) {
                ret << ", ";
            }
            ret << "\"" << kv.first << "\"" << ": " << to_json(make_unique<Value>(move(kv.second->v)));
        }
        ret << "}";
    } else if (holds_alternative<int>(*v)) {
        auto n = move(get<int>(*v));
        ret << n;
    } else if (holds_alternative<string>(*v)) {
        ret << "\"";
        auto s = move(get<string>(*v));
        ret << escape_string(s);
        ret << "\"";
    } else if (holds_alternative<vector<unique_ptr<s>>>(*v)) {
        ret << "[";
        auto vec = move(get<vector<unique_ptr<s>>>(*v));
        auto times_run = 0;
        for (auto& e : vec) {
            if (times_run++) {
                ret << ", ";
            }
            ret << to_json(make_unique<Value>(move(e->v)));
        }
        ret << "]";
    }
    return ret.str();
}

/* auto st = deque<void *>{}; */
auto st = deque<unique_ptr<Value>>{};

enum command {
  create,     // 0
  push_back,  // 1
  number, // 2
  push_string, // 3
  create_map,  // 4
  push_back_map,  // 5
};

static int callout_handler(pcre2_callout_block *c, void *data) {
  cout << "Hello from callout number " << c->callout_number << endl;
  switch (c->callout_number) {
  case create: {
    Value val = vector<unique_ptr<s>> {};
    st.push_back(make_unique<Value>(move(val)));
  } break;
  case push_back: {
    unique_ptr<Value> x = move(st.back());
    st.pop_back();
    unique_ptr<Value> xs = move(st.back());
    vector<unique_ptr<s>> xxss = move(get<vector<unique_ptr<s>>>(*xs));
    unique_ptr<s> unique_s = unique_ptr<s>(new s { .v = move(*x) });
    xxss.push_back(move(unique_s));
    st.push_back(make_unique<Value>(move(xxss)));
  } break;
  case number: {
    int begin_offset = c->offset_vector[c->capture_last * 2];
    int end_offset   = c->offset_vector[c->capture_last * 2 + 1];
    string subject { (char*)c->subject };
    auto val_str = subject.substr(begin_offset, end_offset - begin_offset);
    Value val = stoi(val_str);
    st.push_back(make_unique<Value>(move(val)));
  } break;
  case push_string: {
    int begin_offset = c->offset_vector[c->capture_last * 2];
    int end_offset   = c->offset_vector[c->capture_last * 2 + 1];
    string subject { (char*)c->subject };
    Value val_str = parse_json_string(subject.substr(begin_offset, end_offset - begin_offset));
    st.push_back(make_unique<Value>(move(val_str)));
  } break;
  case create_map: {
    Value m = map<string, unique_ptr<s>> {};
    st.push_back(make_unique<Value>(move(m)));
  } break;
  case push_back_map: {
    unique_ptr<Value> v = move(st.back());
    st.pop_back();
    unique_ptr<Value> k = move(st.back());
    string kk = get<string>(*k);
    st.pop_back();
    unique_ptr<Value> m = move(st.back());
    map<string, unique_ptr<s>> mm = move(get<map<string, unique_ptr<s>>>(*m));
    mm[kk] = unique_ptr<s> { new s { .v = move(*v) } };
    st.push_back(make_unique<Value>(move(mm)));
  } break;
  default: {
    cout << "Exception is exceptional" << endl;
    throw exception{};
  }
  }
  return 0;
}

int main() {
  pcre2_match_context *match_context = pcre2_match_context_create(nullptr);
  pcre2_set_callout(match_context, callout_handler, nullptr);
  stringstream ss;
  ss << "(*NO_AUTO_POSSESS)(*NO_DOTSTAR_ANCHOR)(*NO_START_OPT)"
        /* "\\A (?&digit_list) \\z" */
        "\\A (?&map) \\z"
        "(?(DEFINE)"
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
        "(?<digit_list> (?C" << create << ") ( (?&digit) (?C" << push_back << ") )* )"
        "(?<digit> (\\d) (?C" << number << ") )"
        "(?<map> (?C" << create_map << ") \\{ (?&key) : (?&value) (?C" << push_back_map << ")"
        "( , (?&key) : (?&value) (?C" << push_back_map << "))* \\} )"
        "(?<key> (?&string) )"
        "(?<value> (?&digit) )"
        ")";
  auto ss_str = ss.str();
  auto pattern = reinterpret_cast<PCRE2_SPTR>(ss_str.c_str());
  /* auto subject = reinterpret_cast<PCRE2_SPTR>("123"); */
  auto subject = reinterpret_cast<PCRE2_SPTR>("{\"a\":2,\"b\":4}");
  auto subject_length = static_cast<PCRE2_SIZE>(strlen(reinterpret_cast<const char *>(subject)));

  int errornumber;
  PCRE2_SIZE erroroffset;
  pcre2_code *re = pcre2_compile(
      pattern,                      /* the pattern */
      PCRE2_ZERO_TERMINATED,        /* indicates pattern is zero-terminated */
      PCRE2_EXTENDED, &errornumber, /* for error number */
      &erroroffset,                 /* for error offset */
      nullptr);                     /* use default compile context */

  if (re == nullptr) {
    constexpr int BUF_SZ = 256;
    array<PCRE2_UCHAR, BUF_SZ> buffer {};
    pcre2_get_error_message(errornumber, buffer.data(), buffer.size());
    cout << "PCRE2 compilation failed at offset " << erroroffset << ": "
         << buffer.data() << endl;
    return 1;
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
    switch (rc) {
    case PCRE2_ERROR_NOMATCH:
        cout << "No match" << endl;
      break;
    default:
      cout << "Matching error: " << rc << endl;
      break;
    }
    pcre2_match_data_free(match_data); /* Release memory used for the match */
    pcre2_code_free(re);               /*   data and the compiled pattern. */
    return 1;
  }

  /* Match succeded. Get a pointer to the output vector, where string offsets
  are stored. */

  PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);
  cout << "Match succeeded at offset" << static_cast<int>(ovector[0]) << endl;

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
    return 1;
  }

  /* Show substrings stored in the output vector by number. Obviously, in a real
  application you might want to do things other than print them. */

  for (int i = 0; i < rc; i++) {
    PCRE2_SPTR substring_start = subject + ovector[2 * i];
    PCRE2_SIZE substring_length = ovector[2 * i + 1] - ovector[2 * i];
    /* cout << "%2d: %.*s\n", i, (int)substring_length, (char *)substring_start; */
  }
  cout << to_json(move(st.back())) << endl;
//   auto m = *static_cast<map<int, int> *>(st.back());
//   for (const auto& kv : m) {
//       cout << kv.first << ": " << kv.second << endl;
//   }
//   /* auto v = (*static_cast<vector<int> *>(st.back())); */
//   /* for (int i : v) { */
//   /*     cout << i << endl; */
//   /* } */
  cout << "Still running though" << endl;
  return 0;
}
