#include "gtest/gtest.h"
#include "pcre2_ast.hpp"
#include <utility>
#include <vector>
#include <memory>
#include <variant>

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc ,argv);
  return RUN_ALL_TESTS();
}

TEST(Pcre2AstTest, empty) {
  ASSERT_EQ(from_json(""s), nullptr);
}

TEST(Pcre2AstTest, number) {
  ASSERT_EQ(*from_json("42"s), JsonValue{42.0});
}

TEST(Pcre2AstTest, string) {
  ASSERT_EQ(*from_json("\"foo\""s), JsonValue{"foo"s});
}

TEST(Pcre2AstTest, map) {
  std::map<std::string, std::shared_ptr<s>> got = std::get<std::map<std::string, std::shared_ptr<s>>>(*from_json("{\"foo\":42}"s));
  std::map<std::string, std::shared_ptr<s>> want{{"foo"s, std::make_shared<s>(s{.v = JsonValue{42.0}})}};
  ASSERT_EQ(got["foo"s]->v, want["foo"s]->v);
}

TEST(Pcre2AstTest, array) {
  std::vector<std::shared_ptr<s>> got = std::get<std::vector<std::shared_ptr<s>>>(*from_json("[1,2,3]"s));
  std::vector<std::shared_ptr<s>> want{
      {
      make_shared<s>(s{.v = JsonValue{1.0}}),
      make_shared<s>(s{.v = JsonValue{2.0}}),
      make_shared<s>(s{.v = JsonValue{3.0}}),
      } };
  ASSERT_EQ(got[0]->v, want[0]->v);
  ASSERT_EQ(got[1]->v, want[1]->v);
  ASSERT_EQ(got[2]->v, want[2]->v);
}

TEST(Pcre2AstTest, bool) {
  ASSERT_EQ(*from_json("true"s), JsonValue{true});
}

TEST(Pcre2AstTest, null) {
  ASSERT_EQ(*from_json("null"s), JsonValue{std::monostate{}});
}

TEST(Pcre2AstTest, map_with_many_entries) {
  std::map<std::string, std::shared_ptr<s>> got = std::get<std::map<std::string, std::shared_ptr<s>>>(*from_json("{\"foo\":42,\"bar\":43}"s));
  std::map<std::string, std::shared_ptr<s>> want{
    {"foo"s, std::make_shared<s>(s{.v = JsonValue{42.0}})},
    {"bar"s, std::make_shared<s>(s{.v = JsonValue{43.0}})}
  };
  ASSERT_EQ(got["foo"s]->v, want["foo"s]->v);
  ASSERT_EQ(got["bar"s]->v, want["bar"s]->v);
}

TEST(Pcre2AstTest, map_tested) {
  std::map<std::string, std::shared_ptr<s>> got = std::get<std::map<std::string, std::shared_ptr<s>>>(*from_json("{\"foo\":{\"bar\":42}}"s));
  std::map<std::string, std::shared_ptr<s>> want{
    {"foo"s, std::make_shared<s>(
        s{.v =
            JsonValue{std::map<std::string, std::shared_ptr<s>>{
                {"bar"s, std::make_shared<s>(s{.v = JsonValue{42.0}})}
            }}
         }
        )}};
  auto got_value = std::get<std::map<std::string, std::shared_ptr<s>>>(got["foo"s]->v)["bar"s]->v;
  auto want_value = std::get<std::map<std::string, std::shared_ptr<s>>>(want["foo"s]->v)["bar"s]->v;
  ASSERT_EQ(got_value, want_value);
}
