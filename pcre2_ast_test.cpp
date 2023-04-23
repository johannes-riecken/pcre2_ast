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

TEST(from_json, empty) {
  ASSERT_EQ(from_json(""s), nullptr);
}

TEST(from_json, number) {
  ASSERT_EQ(*from_json("42"s), JsonValue{42.0});
}

TEST(from_json, string) {
  ASSERT_EQ(*from_json("\"foo\""s), JsonValue{"foo"s});
}

TEST(from_json, map) {
  std::map<std::string, std::shared_ptr<s>, std::less<>> got = std::get<std::map<std::string, std::shared_ptr<s>, std::less<>>>(*from_json("{\"foo\":42}"s));
  std::map<std::string, std::shared_ptr<s>, std::less<>> want{{"foo"s, std::make_shared<s>(s{.v = JsonValue{42.0}})}};
  ASSERT_EQ(got["foo"s]->v, want["foo"s]->v);
}

TEST(from_json, array) {
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

TEST(from_json, bool) {
  ASSERT_EQ(*from_json("true"s), JsonValue{true});
}

TEST(from_json, null) {
  ASSERT_EQ(*from_json("null"s), JsonValue{std::monostate{}});
}

TEST(from_json, map_with_many_entries) {
  std::map<std::string, std::shared_ptr<s>, std::less<>> got = std::get<std::map<std::string, std::shared_ptr<s>, std::less<>>>(*from_json("{\"foo\":42,\"bar\":43}"s));
  std::map<std::string, std::shared_ptr<s>, std::less<>> want{
    {"foo"s, std::make_shared<s>(s{.v = JsonValue{42.0}})},
    {"bar"s, std::make_shared<s>(s{.v = JsonValue{43.0}})}
  };
  ASSERT_EQ(got["foo"s]->v, want["foo"s]->v);
  ASSERT_EQ(got["bar"s]->v, want["bar"s]->v);
}

TEST(from_json, map_nested) {
  std::map<std::string, std::shared_ptr<s>, std::less<>> got = std::get<std::map<std::string, std::shared_ptr<s>, std::less<>>>(*from_json("{\"foo\":{\"bar\":42}}"s));
  std::map<std::string, std::shared_ptr<s>, std::less<>> want{
    {"foo"s, std::make_shared<s>(
        s{.v =
            JsonValue{std::map<std::string, std::shared_ptr<s>, std::less<>>{
                {"bar"s, std::make_shared<s>(s{.v = JsonValue{42.0}})}
            }}
         }
        )}};
  auto got_value = std::get<std::map<std::string, std::shared_ptr<s>, std::less<>>>(got["foo"s]->v)["bar"s]->v;
  auto want_value = std::get<std::map<std::string, std::shared_ptr<s>, std::less<>>>(want["foo"s]->v)["bar"s]->v;
  ASSERT_EQ(got_value, want_value);
}

TEST(to_json, number) {
  ASSERT_EQ(to_json(make_shared<JsonValue>(42.0)), "42");
}

TEST(to_json, string) {
  ASSERT_EQ(to_json(make_shared<JsonValue>("foo"s)), "\"foo\"");
}

TEST(to_json, null) {
  ASSERT_EQ(to_json(make_shared<JsonValue>(std::monostate{})), "null");
}

TEST(to_json, true) {
  ASSERT_EQ(to_json(make_shared<JsonValue>(true)), "true");
}

TEST(to_json, map) {
  std::shared_ptr<JsonValue> arg = std::make_shared<JsonValue>(JsonValue{std::map<std::string, std::shared_ptr<s>, std::less<>>{{"foo"s, std::make_shared<s>(s{.v = JsonValue{42.0}})}}});
  ASSERT_EQ(to_json(arg), "{\"foo\": 42}");
}

TEST(to_json, array) {
  std::vector<std::shared_ptr<s>> arg{
      {
      make_shared<s>(s{.v = JsonValue{1.0}}),
      make_shared<s>(s{.v = JsonValue{2.0}}),
      make_shared<s>(s{.v = JsonValue{3.0}}),
      } };
  ASSERT_EQ(to_json(make_shared<JsonValue>(JsonValue{arg})), "[1, 2, 3]");
}
