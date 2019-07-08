#include "../../deps/googletest/include/gtest/gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <time.h>
#include "../../src/datatypes/time_stamp.h"
#include "../../src/util/rmalloc.h"
#include "../../src/value.h"


#ifdef __cplusplus
}
#endif

class TimeStampTest: public ::testing::Test {
  protected:
    static void SetUpTestCase() {// Use the malloc family for allocations
      Alloc_Reset();
    }
};


TEST_F(TimeStampTest, TypeConversions) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    RG_TimeStamp timeStamp = RG_TimeStamp_New_FromTimeSpec(ts);
    ASSERT_EQ(ts.tv_sec, timeStamp.seconds);
    ASSERT_EQ(ts.tv_nsec, timeStamp.nano);

    SIValue value = SI_TimeStamp(&timeStamp);
    ASSERT_EQ(ts.tv_sec, value.longval);
    ASSERT_EQ(ts.tv_nsec, value.allocation);
    ASSERT_EQ(sizeof(int), sizeof(value.allocation));
}