#include "../googletest/include/gtest/gtest.h"
#include "../../src/value.h"
#include "test_value.c"

TEST(ValueTest, ParseTest) {
	printf("testing value parsing:\n");
	test_value_parse();
  /* TODO - This is currently just invoking the tests held in test_value.c
   * A more sensible implementation would take return values from that file
   * and use the assert mechanisms provided by Google Test. */
}