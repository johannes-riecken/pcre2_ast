#include "gtest/gtest.h"
#include "pcre2_ast.hpp"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc ,argv);
  return RUN_ALL_TESTS();
}

TEST(Pcre2AstTest, empty) {
  ASSERT_EQ(from_json(""s), nullptr);
}
