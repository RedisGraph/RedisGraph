#include "../../deps/googletest/include/gtest/gtest.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <time.h>
#include "../../src/datatypes/temporal_value.h"
#include "../../src/util/rmalloc.h"
#include "../../src/value.h"

#ifdef __cplusplus
}
#endif

class TemporalValueTest : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    { // Use the malloc family for allocations
        Alloc_Reset();
    }
};

TEST_F(TemporalValueTest, TypeConversions)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    RG_TemporalValue temporalValue = RG_DateTime_New_FromTimeSpec(ts);
    ASSERT_EQ(ts.tv_sec, temporalValue.seconds);
    ASSERT_EQ(ts.tv_nsec, temporalValue.nano);

    SIValue value = SI_TemporalValue(temporalValue);
    ASSERT_EQ(ts.tv_sec, value.longval);
    ASSERT_EQ(ts.tv_nsec, value.allocation);
    ASSERT_EQ(sizeof(int), sizeof(value.allocation));
}

struct tm makeNewBrokenTime()
{
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

struct timespec createTimespec(struct tm t)
{
    time_t epoch = timegm(&t);
    struct timespec ts;
    ts.tv_sec = epoch;
    ts.tv_nsec = 0;
    return ts;
}

TEST_F(TemporalValueTest, TestYear)
{
    struct tm t = makeNewBrokenTime();
    t.tm_year = 2019 - 1900;
    struct timespec ts = createTimespec(t);

    // check for datetime
    RG_TemporalValue dateTime = RG_DateTime_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetYear(dateTime), 2019);

    // check for local dateTime
    RG_TemporalValue localDateTime = RG_LocalDateTime_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetYear(localDateTime), 2019);

    // check for date
    RG_TemporalValue date = RG_Date_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetYear(date), 2019);

    // check for time
    RG_TemporalValue time = RG_Time_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetYear(time), RG_TEMPORAL_FUNC_FAIL);

    // check for local time
    RG_TemporalValue localTime = RG_LocalTime_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetYear(localTime), RG_TEMPORAL_FUNC_FAIL);
}

TEST_F(TemporalValueTest, TestQuarter)
{
    for (int q = 0; q < 4; q++)
    {
        for (int m = 1; m < 3; m++)
        {
            struct tm t = makeNewBrokenTime();
            t.tm_mon = 3 * q + m;
            struct timespec ts = createTimespec(t);

            // check for date time
            RG_TemporalValue dateTime = RG_DateTime_New_FromTimeSpec(ts);
            ASSERT_EQ(RG_TemporalValue_GetQuarter(dateTime), q + 1);

            // check for local dateTime
            RG_TemporalValue localDateTime = RG_LocalDateTime_New_FromTimeSpec(ts);
            ASSERT_EQ(RG_TemporalValue_GetQuarter(localDateTime), q + 1);

            // check for date
            RG_TemporalValue date = RG_Date_New_FromTimeSpec(ts);
            ASSERT_EQ(RG_TemporalValue_GetQuarter(date), q + 1);

            // check for time
            RG_TemporalValue time = RG_Time_New_FromTimeSpec(ts);
            ASSERT_EQ(RG_TemporalValue_GetQuarter(time), RG_TEMPORAL_FUNC_FAIL);

            // check for local time
            RG_TemporalValue localTime = RG_LocalTime_New_FromTimeSpec(ts);
            ASSERT_EQ(RG_TemporalValue_GetQuarter(localTime), RG_TEMPORAL_FUNC_FAIL);
        }
    }
}

TEST_F(TemporalValueTest, TestMonth)
{
    for (int m = 0; m < 12; m++)
    {
        struct tm t = makeNewBrokenTime();
        t.tm_mon = m;
        struct timespec ts = createTimespec(t);

        // check for date time
        RG_TemporalValue dateTime = RG_DateTime_New_FromTimeSpec(ts);
        ASSERT_EQ(RG_TemporalValue_GetMonth(dateTime), m + 1);

        // check for local dateTime
        RG_TemporalValue localDateTime = RG_LocalDateTime_New_FromTimeSpec(ts);
        ASSERT_EQ(RG_TemporalValue_GetMonth(localDateTime), m + 1);

        // check for date
        RG_TemporalValue date = RG_Date_New_FromTimeSpec(ts);
        ASSERT_EQ(RG_TemporalValue_GetMonth(date), m + 1);

        // check for time
        RG_TemporalValue time = RG_Time_New_FromTimeSpec(ts);
        ASSERT_EQ(RG_TemporalValue_GetMonth(time), RG_TEMPORAL_FUNC_FAIL);

        // check for local time
        RG_TemporalValue localTime = RG_LocalTime_New_FromTimeSpec(ts);
        ASSERT_EQ(RG_TemporalValue_GetMonth(localTime), RG_TEMPORAL_FUNC_FAIL);
    }
}

TEST_F(TemporalValueTest, TestWeek)
{
    struct tm t = makeNewBrokenTime();
    t.tm_year = 2008 - 1900;
    t.tm_mon = 12 - 1;
    t.tm_mday = 31;
    t.tm_isdst = -1;
    struct timespec ts = createTimespec(t);

    // check for date time
    RG_TemporalValue dateTime = RG_DateTime_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetWeek(dateTime), 1);

    // check for local dateTime
    RG_TemporalValue localDateTime = RG_LocalDateTime_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetWeek(localDateTime), 1);

    // check for date
    RG_TemporalValue date = RG_Date_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetWeek(date), 1);

    // check for time
    RG_TemporalValue time = RG_Time_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetWeek(time), RG_TEMPORAL_FUNC_FAIL);

    // check for local time
    RG_TemporalValue localTime = RG_LocalTime_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetWeek(localTime), RG_TEMPORAL_FUNC_FAIL);
}

TEST_F(TemporalValueTest, TestWeekYear)
{
    struct tm t = makeNewBrokenTime();
    t.tm_year = 2008 - 1900;
    t.tm_mon = 12 - 1;
    t.tm_mday = 31;
    t.tm_isdst = -1;
    struct timespec ts = createTimespec(t);

    // check for date time
    RG_TemporalValue dateTime = RG_DateTime_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetWeekYear(dateTime), 2009);

    // check for local dateTime
    RG_TemporalValue localDateTime = RG_LocalDateTime_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetWeekYear(localDateTime), 2009);

    // check for date
    RG_TemporalValue date = RG_Date_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetWeekYear(date), 2009);

    // check for time
    RG_TemporalValue time = RG_Time_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetWeekYear(time), RG_TEMPORAL_FUNC_FAIL);

    // check for local time
    RG_TemporalValue localTime = RG_LocalTime_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetWeekYear(localTime), RG_TEMPORAL_FUNC_FAIL);
}

TEST_F(TemporalValueTest, TestDayOfQuarter)
{
    struct tm t = makeNewBrokenTime();
    t.tm_year = 2020 - 1900;
    t.tm_mon = 1;
    t.tm_mday = 29;
    t.tm_isdst = -1;
    struct timespec ts = createTimespec(t);

    // check for date time
    RG_TemporalValue dateTime = RG_DateTime_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetDayOfQuarter(dateTime), 60);

    // check for local dateTime
    RG_TemporalValue localDateTime = RG_LocalDateTime_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetDayOfQuarter(localDateTime), 60);

    // check for date
    RG_TemporalValue date = RG_Date_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetDayOfQuarter(date), 60);

    // check for time
    RG_TemporalValue time = RG_Time_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetDayOfQuarter(time), RG_TEMPORAL_FUNC_FAIL);

    // check for local time
    RG_TemporalValue localTime = RG_LocalTime_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetDayOfQuarter(localTime), RG_TEMPORAL_FUNC_FAIL);
}

TEST_F(TemporalValueTest, TestDay)
{
    struct tm t = makeNewBrokenTime();
    t.tm_year = 2020 - 1900;
    t.tm_mon = 1;
    t.tm_mday = 29;
    t.tm_isdst = -1;
    struct timespec ts = createTimespec(t);

    // check for date time
    RG_TemporalValue dateTime = RG_DateTime_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetMonth(dateTime), 2);
    ASSERT_EQ(RG_TemporalValue_GetDay(dateTime), 29);

    // check for local dateTime
    RG_TemporalValue localDateTime = RG_LocalDateTime_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetMonth(localDateTime), 2);
    ASSERT_EQ(RG_TemporalValue_GetDay(localDateTime), 29);

    // check for date
    RG_TemporalValue date = RG_Date_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetMonth(date), 2);
    ASSERT_EQ(RG_TemporalValue_GetDay(date), 29);

    // check for time
    RG_TemporalValue time = RG_Time_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetMonth(time), RG_TEMPORAL_FUNC_FAIL);
    ASSERT_EQ(RG_TemporalValue_GetDay(time), RG_TEMPORAL_FUNC_FAIL);

    // check for local time
    RG_TemporalValue localTime = RG_LocalTime_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetMonth(time), RG_TEMPORAL_FUNC_FAIL);
    ASSERT_EQ(RG_TemporalValue_GetDay(time), RG_TEMPORAL_FUNC_FAIL);
}

TEST_F(TemporalValueTest, TestOrdinalDay)
{
    struct tm t = makeNewBrokenTime();
    t.tm_year = 2020 - 1900;
    t.tm_mon = 1;
    t.tm_mday = 29;
    t.tm_isdst = -1;
    struct timespec ts = createTimespec(t);

    // check for date time
    RG_TemporalValue dateTime = RG_DateTime_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetMonth(dateTime), 2);
    ASSERT_EQ(RG_TemporalValue_GetDay(dateTime), 29);
    ASSERT_EQ(RG_TemporalValue_GetOrdinalDay(dateTime), 60);

    // check for local dateTime
    RG_TemporalValue localDateTime = RG_LocalDateTime_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetMonth(localDateTime), 2);
    ASSERT_EQ(RG_TemporalValue_GetDay(localDateTime), 29);
    ASSERT_EQ(RG_TemporalValue_GetOrdinalDay(localDateTime), 60);

    // check for date
    RG_TemporalValue date = RG_Date_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetMonth(date), 2);
    ASSERT_EQ(RG_TemporalValue_GetDay(date), 29);
    ASSERT_EQ(RG_TemporalValue_GetOrdinalDay(date), 60);

    // check for time
    RG_TemporalValue time = RG_Time_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetMonth(time), RG_TEMPORAL_FUNC_FAIL);
    ASSERT_EQ(RG_TemporalValue_GetDay(time), RG_TEMPORAL_FUNC_FAIL);
    ASSERT_EQ(RG_TemporalValue_GetOrdinalDay(time), RG_TEMPORAL_FUNC_FAIL);

    // check for local time
    RG_TemporalValue localTime = RG_LocalTime_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetMonth(localTime), RG_TEMPORAL_FUNC_FAIL);
    ASSERT_EQ(RG_TemporalValue_GetDay(localTime), RG_TEMPORAL_FUNC_FAIL);
    ASSERT_EQ(RG_TemporalValue_GetOrdinalDay(localTime), RG_TEMPORAL_FUNC_FAIL);
}

TEST_F(TemporalValueTest, TestDayOfWeek)
{
    for (int i = 1; i <= 31; i++)
    {
        struct tm t = makeNewBrokenTime();
        t.tm_year = 2019 - 1900;
        t.tm_mon = 6;
        t.tm_mday = i;
        t.tm_isdst = -1;
        struct timespec ts = createTimespec(t);

        // check for datetime
        RG_TemporalValue dateTime = RG_DateTime_New_FromTimeSpec(ts);
        ASSERT_EQ(RG_TemporalValue_GetDayOfWeek(dateTime), (i % 7) == 0 ? 7 : i % 7);

        // check for local dateTime
        RG_TemporalValue localDateTime = RG_LocalDateTime_New_FromTimeSpec(ts);
        ASSERT_EQ(RG_TemporalValue_GetDayOfWeek(localDateTime), (i % 7) == 0 ? 7 : i % 7);

        // check for date
        RG_TemporalValue date = RG_Date_New_FromTimeSpec(ts);
        ASSERT_EQ(RG_TemporalValue_GetDayOfWeek(date), (i % 7) == 0 ? 7 : i % 7);

        // check for time
        RG_TemporalValue time = RG_Time_New_FromTimeSpec(ts);
        ASSERT_EQ(RG_TemporalValue_GetDayOfWeek(time), RG_TEMPORAL_FUNC_FAIL);

        // check for local time
        RG_TemporalValue localTime = RG_LocalTime_New_FromTimeSpec(ts);
        ASSERT_EQ(RG_TemporalValue_GetDayOfWeek(localTime), RG_TEMPORAL_FUNC_FAIL);
    }
}

TEST_F(TemporalValueTest, TestTime)
{
    struct tm t = makeNewBrokenTime();
    t.tm_sec = 41;
    t.tm_min = 37;
    t.tm_hour = 14;
    struct timespec ts = createTimespec(t);
    ts.tv_nsec = 123456789;
    // check for datetime
    RG_TemporalValue dateTime = RG_DateTime_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetHour(dateTime), 14);
    ASSERT_EQ(RG_TemporalValue_GetMinute(dateTime), 37);
    ASSERT_EQ(RG_TemporalValue_GetSecond(dateTime), 41);
    ASSERT_EQ(RG_TemporalValue_GetMillisecond(dateTime), 123);
    ASSERT_EQ(RG_TemporalValue_GetMicrosecond(dateTime), 123456);
    ASSERT_EQ(RG_TemporalValue_GetNanosecond(dateTime), 123456789);
    // check for local dateTime
    RG_TemporalValue localDateTime = RG_LocalDateTime_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetHour(localDateTime), 14);
    ASSERT_EQ(RG_TemporalValue_GetMinute(localDateTime), 37);
    ASSERT_EQ(RG_TemporalValue_GetSecond(localDateTime), 41);
    ASSERT_EQ(RG_TemporalValue_GetMillisecond(localDateTime), 123);
    ASSERT_EQ(RG_TemporalValue_GetMicrosecond(localDateTime), 123456);
    ASSERT_EQ(RG_TemporalValue_GetNanosecond(localDateTime), 123456789);
    // check for time
    RG_TemporalValue time = RG_Time_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetHour(time), 14);
    ASSERT_EQ(RG_TemporalValue_GetMinute(time), 37);
    ASSERT_EQ(RG_TemporalValue_GetSecond(time), 41);
    ASSERT_EQ(RG_TemporalValue_GetMillisecond(time), 123);
    ASSERT_EQ(RG_TemporalValue_GetMicrosecond(time), 123456);
    ASSERT_EQ(RG_TemporalValue_GetNanosecond(time), 123456789);

    // check for local time
    RG_TemporalValue localTime = RG_LocalTime_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetHour(localTime), 14);
    ASSERT_EQ(RG_TemporalValue_GetMinute(localTime), 37);
    ASSERT_EQ(RG_TemporalValue_GetSecond(localTime), 41);
    ASSERT_EQ(RG_TemporalValue_GetMillisecond(localTime), 123);
    ASSERT_EQ(RG_TemporalValue_GetMicrosecond(localTime), 123456);
    ASSERT_EQ(RG_TemporalValue_GetNanosecond(localTime), 123456789);

    // check for date
    RG_TemporalValue date = RG_Date_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetHour(date), RG_TEMPORAL_FUNC_FAIL);
    ASSERT_EQ(RG_TemporalValue_GetMinute(date), RG_TEMPORAL_FUNC_FAIL);
    ASSERT_EQ(RG_TemporalValue_GetSecond(date), RG_TEMPORAL_FUNC_FAIL);
    ASSERT_EQ(RG_TemporalValue_GetMillisecond(date), RG_TEMPORAL_FUNC_FAIL);
    ASSERT_EQ(RG_TemporalValue_GetMicrosecond(date), RG_TEMPORAL_FUNC_FAIL);
    ASSERT_EQ(RG_TemporalValue_GetNanosecond(date), RG_TEMPORAL_FUNC_FAIL);
}

// TEST_F(TimeStampTest, TestPrint) {

//     RG_TimeStamp temporalValue = RG_TimeStamp_New();
//     std::cout << RG_TimeStamp_ToString(temporalValue) << std::endl;
// }