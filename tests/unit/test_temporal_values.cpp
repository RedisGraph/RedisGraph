#include "../../deps/googletest/include/gtest/gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <time.h>
#include "../../src/datatypes/temporal_value.h"
#include "../../src/util/rmalloc.h"
#include "../../src/value.h"


#ifdef __cplusplus
}
#endif

class TemporalValueTest: public ::testing::Test {
  protected:
	static void SetUpTestCase() {// Use the malloc family for allocations
		Alloc_Reset();
	}
};


TEST_F(TemporalValueTest, TypeConversions) {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	RG_TemporalValue temporalValue = RG_DateTime_New_FromTimeSpec(ts);
	ASSERT_EQ(ts.tv_sec, temporalValue.seconds);
	ASSERT_EQ(ts.tv_nsec, temporalValue.nano);

	SIValue value = SI_TemporalValue(&temporalValue);
	ASSERT_EQ(ts.tv_sec, value.longval);
	ASSERT_EQ(ts.tv_nsec, value.allocation);
	ASSERT_EQ(sizeof(int), sizeof(value.allocation));
}

struct tm makeNewBrokenTime() {
	struct tm t;
	t.tm_year = 0;
	t.tm_sec = 0;
	t.tm_min = 0;
	t.tm_hour = 0;
	t.tm_mday = 1;
	t.tm_mon = 0;
	t.tm_yday = 0;
	t.tm_isdst = 0;
	t.tm_gmtoff = 0;
	t.tm_wday = 0;
	t.tm_zone = "UTC";
	return t;
}

struct timespec createTimespec(struct tm t) {
	time_t epoch = timegm(&t);
	struct timespec ts;
	ts.tv_sec = epoch;
	ts.tv_nsec = 0;
	return ts;
}

TEST_F(TemporalValueTest, TestYear) {
	struct tm t = makeNewBrokenTime();
	t.tm_year = 119;
	struct timespec ts = createTimespec(t);
	RG_TemporalValue dateTime = RG_DateTime_New_FromTimeSpec(ts);
	ASSERT_EQ(RG_TemporalValue_GetYear(dateTime), 2019);
}

TEST_F(TemporalValueTest, TestQuarter) {
	for(int q = 0; q < 4; q++) {
		for(int m = 1; m < 3; m++) {
			struct tm t = makeNewBrokenTime();
			t.tm_mon = 3 * q + m;
			struct timespec ts = createTimespec(t);
			RG_TemporalValue dateTime = RG_DateTime_New_FromTimeSpec(ts);
			ASSERT_EQ(RG_TemporalValue_GetQuarter(dateTime), q + 1);
		}
	}
}

TEST_F(TemporalValueTest, TestMonth) {
	for(int m = 0; m < 12; m++) {
		struct tm t = makeNewBrokenTime();
		t.tm_mon = m;
		struct timespec ts = createTimespec(t);
		RG_TemporalValue dateTime = RG_DateTime_New_FromTimeSpec(ts);
		ASSERT_EQ(RG_TemporalValue_GetMonth(dateTime), m + 1);
	}
}

TEST_F(TemporalValueTest, TestWeek) {
	struct tm t = makeNewBrokenTime();
	t.tm_year = 2008 - 1900;
	t.tm_mon = 12 - 1;
	t.tm_mday = 31;
	t.tm_isdst = -1;
	struct timespec ts = createTimespec(t);
	RG_TemporalValue dateTime = RG_DateTime_New_FromTimeSpec(ts);
	ASSERT_EQ(1, RG_TemporalValue_GetWeek(dateTime));
}

TEST_F(TemporalValueTest, TestWeekYear) {
	struct tm t = makeNewBrokenTime();
	t.tm_year = 2008 - 1900;
	t.tm_mon = 12 - 1;
	t.tm_mday = 31;
	t.tm_isdst = -1;
	struct timespec ts = createTimespec(t);
	RG_TemporalValue dateTime = RG_DateTime_New_FromTimeSpec(ts);
	ASSERT_EQ(2009, RG_TemporalValue_GetWeekYear(dateTime));
}

TEST_F(TemporalValueTest, TestDayOfQuarter) {
	struct tm t = makeNewBrokenTime();
	t.tm_year = 2020;
	t.tm_mon = 1;
	t.tm_mday = 29;
	t.tm_isdst = -1;
	struct timespec ts = createTimespec(t);
	RG_TemporalValue dateTime = RG_DateTime_New_FromTimeSpec(ts);
	ASSERT_EQ(60, RG_TemporalValue_GetDayOfQuarter(dateTime));
}



// TEST_F(TimeStampTest, TestPrint) {

//     RG_TimeStamp temporalValue = RG_TimeStamp_New();
//     std::cout << RG_TimeStamp_ToString(temporalValue) << std::endl;
// }