// Inspired by https://www.perlmonks.org/?node_id=995856
#define PCRE2_CODE_UNIT_WIDTH 8

#include <array>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stack>
#include <string>
#include <variant>
#include <vector>

#include <pcre2.h>
#include <queue>
#include <unistd.h>

#include "pcre2_ast.hpp"

constexpr bool is_debug = false;

using namespace std;


enum command {  // comments are factor expressions
    create_array,  // { }
    push_back_array,  // suffix
    push_number,  // 42
    push_string,  // "string"
    create_map,  //  H{ }
    push_back_map,  // ?set-at value key assoc/f
    push_true,  // t
    push_false,  // f
    push_null,  // null
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
        default:
            __builtin_trap();
    }
}

string escape_string(const string &s) {
    stringstream ret{};
    ret << "\"";
    for (char c: s) {
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
    stringstream ret{};
    s = s.substr(1, s.size() - 2); // remove quotes

    bool after_backslash = false;
    for (char c: s) {
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
                    throw exception{};
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


string to_json(const shared_ptr<JsonValue> &v) {
    using QueueItem = variant<JsonValue, string>;
    queue<QueueItem> q;
    q.push(*v);
    stringstream ret{};
    ret << fixed << setprecision(0);
    while (!q.empty()) {
        auto top = q.front();
        q.pop();
        if (holds_alternative<string>(top)) {
            auto str = get<string>(top);
            ret << str;
            continue;
        }
        JsonValue cur = get<JsonValue>(top);
        if (holds_alternative<map<string, shared_ptr<s>>>(cur)) {
            ret << "{";
            auto m = move(get<map<string, shared_ptr<s>>>(cur));
            auto times_run = 0;
            for (auto &kv: m) {
                if (times_run++) {
                    q.push(", "s);
                }
                q.push("\"" + kv.first + "\"" + ": ");
                q.push((kv.second->v));
            }
            q.push("}"s);
        } else if (holds_alternative<double>(cur)) {
            auto n = get<double>(cur);
            ret << n;
        } else if (holds_alternative<string>(cur)) {
            auto str = move(get<string>(cur));
            ret << escape_string(str);
        } else if (holds_alternative<vector<shared_ptr<s>>>(cur)) {
            ret << "[";
            auto vec = move(get<vector<shared_ptr<s>>>(cur));
            auto times_run = 0;
            for (auto &e: vec) {
                if (times_run++) {
                    q.push(", "s);
                }
                q.push((e->v));
            }
            q.push("]"s);
        } else if (holds_alternative<bool>(cur)) {
            auto b = get<bool>(cur);
            ret << (b ? "true" : "false");
        } else if (holds_alternative<monostate>(cur)) {
            ret << "null";
        }
    }
    return ret.str();
}

auto st = deque<shared_ptr<JsonValue>>{};

static int callout_handler(pcre2_callout_block *c, [[maybe_unused]] void *data) {
    if (is_debug) {
        cout << command_to_string(static_cast<command>(c->callout_number)) << endl;
    }
    switch (c->callout_number) {
        case create_array: {
            JsonValue val = vector<shared_ptr<s>>{};
            st.push_back(make_shared<JsonValue>(move(val)));
        }
            break;
        case push_back_array: {
            shared_ptr<JsonValue> x = move(st.back());
            st.pop_back();
            shared_ptr<JsonValue> vec_variant = move(st.back());
            st.pop_back();
            vector<shared_ptr<s>> vec = move(get<vector<shared_ptr<s>>>(*vec_variant));
            vec.push_back(shared_ptr<s>(new s{.v = move(*x)}));
            st.push_back(make_shared<JsonValue>(move(vec)));
        }
            break;
        case push_number: {
            auto begin_offset = c->offset_vector[c->capture_last * 2];
            auto end_offset = c->offset_vector[c->capture_last * 2 + 1];
            string subject{(char *) c->subject};
            auto val_str = subject.substr(begin_offset, end_offset - begin_offset);
            st.push_back(make_shared<JsonValue>(stod(val_str)));
        }
            break;
        case push_string: {
            auto begin_offset = c->offset_vector[c->capture_last * 2];
            auto end_offset = c->offset_vector[c->capture_last * 2 + 1];
            string subject{(char *) c->subject};
            JsonValue val_str = parse_json_string(subject.substr(begin_offset, end_offset - begin_offset));
            st.push_back(make_shared<JsonValue>(move(val_str)));
        }
            break;
        case create_map: {
            st.push_back(make_shared<JsonValue>(map<string, shared_ptr<s>>{}));
        }
            break;
        case push_back_map: {
            shared_ptr<JsonValue> v = move(st.back());
            st.pop_back();
            string k = get<string>(*move(st.back()));
            st.pop_back();
            map<string, shared_ptr<s>> m = move(get<map<string, shared_ptr<s>>>(*st.back()));
            st.pop_back();
            m[k] = shared_ptr<s>{new s{.v = move(*v)}};
            st.push_back(make_shared<JsonValue>(move(m)));
        }
            break;
        case push_true: {
            st.push_back(move(make_shared<JsonValue>(true)));
        }
            break;
        case push_false: {
            st.push_back(move(make_shared<JsonValue>(false)));
        }
            break;
        case push_null: {
            st.push_back(move(make_shared<JsonValue>(monostate{})));
        }
            break;
        default: {
            cout << "Exception is exceptional" << endl;
            throw exception{};
        }
    }
    return 0;
}


shared_ptr<JsonValue> from_json(const string &str) {
    pcre2_match_context *match_context = pcre2_match_context_create(nullptr);
    pcre2_set_callout(match_context, callout_handler, nullptr);
    ifstream regex_file("regex.txt");
    if (!regex_file.is_open()) {
        cout << "Failed to open regex.txt" << endl;
        throw exception{};
    }
    string regex_str((istreambuf_iterator<char>(regex_file)), istreambuf_iterator<char>());
    auto pattern = reinterpret_cast<PCRE2_SPTR>(regex_str.c_str());
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
        array<PCRE2_UCHAR, BUF_SZ> buffer{};
        pcre2_get_error_message(errornumber, buffer.data(), buffer.size());
        /* cout << "PCRE2 compilation failed at offset " << erroroffset << ": " */
        /*      << buffer.data() << endl; */
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
            /* cout << "No match" << endl; */
        } else {
            /* cout << "Matching error: " << rc << endl; */
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
        /* cout << "ovector was not big enough for all the captured substrings" << endl; */
    }

    /* We must guard against patterns such as /(?=.\K)/ that use \K in an
    assertion to set the start of a match later than its end. In this
    demonstration program, we just detect this case and give up. */

    if (ovector[0] > ovector[1]) {
        /* cout << */
        /*     "\\K was used in an assertion to set the match start after its end.\n"; */
        /*     /1* "From end to start the match was: %.*s\n", *1/ */
        /*     /1* (int)(ovector[0] - ovector[1]), (char *)(subject + ovector[1]); *1/ */
        /* cout << "Run abandoned" << endl; */
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
