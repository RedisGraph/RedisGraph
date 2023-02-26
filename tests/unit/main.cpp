
#include "gtest/gtest.h"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  //::testing::AddGlobalTestEnvironment(new MyEnvironment());
  return RUN_ALL_TESTS();
}
