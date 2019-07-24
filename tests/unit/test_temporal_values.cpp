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

    SIValue value = SI_TemporalValue(&temporalValue);
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
    ASSERT_EQ(RG_TemporalValue_GetYear(time), INT_MIN);

    // check for local time
    RG_TemporalValue localTime = RG_LocalTime_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetYear(localTime), INT_MIN);
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
            ASSERT_EQ(RG_TemporalValue_GetQuarter(time), INT_MIN);

            // check for local time
            RG_TemporalValue localTime = RG_LocalTime_New_FromTimeSpec(ts);
            ASSERT_EQ(RG_TemporalValue_GetQuarter(localTime), INT_MIN);
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
        ASSERT_EQ(RG_TemporalValue_GetMonth(time), INT_MIN);

        // check for local time
        RG_TemporalValue localTime = RG_LocalTime_New_FromTimeSpec(ts);
        ASSERT_EQ(RG_TemporalValue_GetMonth(localTime), INT_MIN);
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
    ASSERT_EQ(RG_TemporalValue_GetWeek(time), INT_MIN);

    // check for local time
    RG_TemporalValue localTime = RG_LocalTime_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetWeek(localTime), INT_MIN);
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
    ASSERT_EQ(RG_TemporalValue_GetWeekYear(time), INT_MIN);

    // check for local time
    RG_TemporalValue localTime = RG_LocalTime_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetWeekYear(localTime), INT_MIN);
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
    ASSERT_EQ(RG_TemporalValue_GetDayOfQuarter(time), INT_MIN);

    // check for local time
    RG_TemporalValue localTime = RG_LocalTime_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetDayOfQuarter(localTime), INT_MIN);
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
    ASSERT_EQ(RG_TemporalValue_GetMonth(time), INT_MIN);
    ASSERT_EQ(RG_TemporalValue_GetDay(time), INT_MIN);

    // check for local time
    RG_TemporalValue localTime = RG_LocalTime_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetMonth(time), INT_MIN);
    ASSERT_EQ(RG_TemporalValue_GetDay(time), INT_MIN);
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
    ASSERT_EQ(RG_TemporalValue_GetMonth(time), INT_MIN);
    ASSERT_EQ(RG_TemporalValue_GetDay(time), INT_MIN);
    ASSERT_EQ(RG_TemporalValue_GetOrdinalDay(time), INT_MIN);

    // check for local time
    RG_TemporalValue localTime = RG_LocalTime_New_FromTimeSpec(ts);
    ASSERT_EQ(RG_TemporalValue_GetMonth(time), INT_MIN);
    ASSERT_EQ(RG_TemporalValue_GetDay(time), INT_MIN);
    ASSERT_EQ(RG_TemporalValue_GetOrdinalDay(time), INT_MIN);
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
        ASSERT_EQ(RG_TemporalValue_GetDayOfWeek(time), INT_MIN);

        // check for local time
        RG_TemporalValue localTime = RG_LocalTime_New_FromTimeSpec(ts);
        ASSERT_EQ(RG_TemporalValue_GetDayOfWeek(localTime), INT_MIN);
    }
}

// TEST_F(TimeStampTest, TestPrint) {

//     RG_TimeStamp temporalValue = RG_TimeStamp_New();
//     std::cout << RG_TimeStamp_ToString(temporalValue) << std::endl;
// }