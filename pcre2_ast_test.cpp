#include "gtest/gtest.h"
#include "pcre2_ast.hpp"
#include <utility>

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

/* TEST(Pcre2AstTest, map) { */
/*   ASSERT_EQ(*from_json("{\"foo\":42}"s), */
/*       std::map<std::string, std::shared_ptr<s>>{{"foo"s, std::make_shared<s>(s{JsonValue{42}})}}); */
/* } */

/* TEST(Pcre2AstTest, array) { */
/*   ASSERT_EQ(*from_json("[1,2,3]"s), (JsonValue{std::vector<std::shared_ptr<s>>{ */
/*       { */
/*       make_shared<s>(s{.v = JsonValue{1.0}}), */
/*       make_shared<s>(s{.v = JsonValue{2.0}}), */
/*       make_shared<s>(s{.v = JsonValue{3.0}}), */
/*       } }})); */
/* } */

TEST(Pcre2AstTest, bool) {
  ASSERT_EQ(*from_json("true"s), JsonValue{true});
}

TEST(Pcre2AstTest, null) {
  ASSERT_EQ(*from_json("null"s), JsonValue{std::monostate{}});
}
