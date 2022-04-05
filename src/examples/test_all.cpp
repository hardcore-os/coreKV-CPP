#include "test_frame.h"

int Add(int a, int b) { return a + b; }
int Sub(int a, int b) { return a - b; }
TEST(AddTestDemo, Add) {
  EXPECT_EQ(3, Add(1, 2));
  EXPECT_EQ(2, Add(1, 1));
}

TEST(SubTestDemo, Sub) {
  EXPECT_EQ(3, Sub(1, 2));
  EXPECT_EQ(2, Sub(1, 1));
}
int main() { RUN_ALL_TESTS(); }