#include "IntersectionDetection.h"
#include "gtest/gtest.h"

// Anonymous namespaces are a C++ idiom for declaring symbols to be
// file-private, ie not exported, like declaring a function static in straight C.
namespace {

// The fixture for testing intersections.
class IntersectionDetectionTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  IntersectionDetectionTest() {
    // You can do set-up work for each test here.
  }

  virtual ~IntersectionDetectionTest() {
    // You can do clean-up work that doesn't throw exceptions here.
  }
};

TEST_F(IntersectionDetectionTest, crossingLinesIntersect) {
  Vec p1(0, 0);
  Vec p2(1, 1);
  Vec p3(1, 0);
  Vec p4(0, 1);
  EXPECT_TRUE(intersectLines(p1, p2, p3, p4));
}

TEST_F(IntersectionDetectionTest, parallelLinesDontIntersect) {
  Vec p1(0, 0);
  Vec p2(1, 0);
  Vec p3(0, 1);
  Vec p4(1, 1);
  EXPECT_FALSE(intersectLines(p1, p2, p3, p4));
}

}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
