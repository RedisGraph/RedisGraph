/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

typedef enum
{
    TIME = 1,
    LOCAL_TIME = 2,
    DATE = 4,
    DATE_TIME = 8,
    LOCAL_DATE_TIME = 16
} RG_TemporalType;

typedef struct
{
    time_t seconds;             // holds nano seconds from epoch
    uint32_t nano;              // holds nano seconds delta
    unsigned int type : 5;      // holds type
    unsigned int timeZone : 27; // bitmap to hold temporal type and timezone id
} RG_TemporalValue;

// new temporal values
RG_TemporalValue RG_Date_New();
RG_TemporalValue RG_Time_New();
RG_TemporalValue RG_LocalTime_New();
RG_TemporalValue RG_DateTime_New();
RG_TemporalValue RG_LocalDateTime_New();

// TODO: testing & future feature - build new temporal values from map
RG_TemporalValue RG_Date_New_FromTimeSpec(struct timespec ts);
RG_TemporalValue RG_Time_New_FromTimeSpec(struct timespec ts);
RG_TemporalValue RG_LocalTime_New_FromTimeSpec(struct timespec ts);
RG_TemporalValue RG_DateTime_New_FromTimeSpec(struct timespec ts);
RG_TemporalValue RG_LocalDateTime_New_FromTimeSpec(struct timespec ts);

// TODO: future feature - build from string
RG_TemporalValue RG_Date_New_FromString(const char *str);
RG_TemporalValue RG_Time_New_FromString(const char *str);
RG_TemporalValue RG_LocalTime_New_FromString(const char *str);
RG_TemporalValue RG_DateTime_New_FromString(const char *str);
RG_TemporalValue RG_LocalDateTime_New_FromString(const char *str);

int64_t RG_TemporalValue_GetYear(RG_TemporalValue timeValue);
int64_t RG_TemporalValue_GetQuarter(RG_TemporalValue timeValue);
int64_t RG_TemporalValue_GetMonth(RG_TemporalValue temporalValue);
int64_t RG_TemporalValue_GetWeek(RG_TemporalValue temporalValue);
int64_t RG_TemporalValue_GetWeekYear(RG_TemporalValue temporalValue);
int64_t RG_TemporalValue_GetDayOfQuarter(RG_TemporalValue temporalValue);
int64_t RG_TemporalValue_GetDay(RG_TemporalValue temporalValue);
int64_t RG_TemporalValue_GetOrdinalDay(RG_TemporalValue temporalValue);
int64_t RG_TemporalValue_GetDayOfWeek(RG_TemporalValue temporalValue);
int64_t RG_TemporalValue_GetHour(RG_TemporalValue temporalValue);
int64_t RG_TemporalValue_GetMinute(RG_TemporalValue temporalValue);
int64_t RG_TemporalValue_GetSecond(RG_TemporalValue temporalValue);
int64_t RG_TemporalValue_GetMillisecond(RG_TemporalValue temporalValue);
int64_t RG_TemporalValue_GetMicrosecond(RG_TemporalValue temporalValue);
int64_t RG_TemporalValue_GetNanosecond(RG_TemporalValue temporalValue);
// const char* RG_TemporalValue_GetTimeZone(RG_TemporalValue temporalValue);
// const char* RG_TemporalValue_GetOffset(RG_TemporalValue temporalValue);
// int64_t RG_TemporalValue_GetOffsetMinuets(RG_TemporalValue temporalValue);
// int64_t RG_TemporalValue_GetOffsetSeconds(RG_TemporalValue temporalValue);
// int64_t RG_TemporalValue_GetEpochMillis(RG_TemporalValue temporalValue);
// int64_t RG_TemporalValue_GetEpochSeconds(RG_TemporalValue temporalValue);

const char *RG_TemporalValue_ToString(RG_TemporalValue timestamp);
int RG_TemporalValue_Compare(RG_TemporalValue a, RG_TemporalValue b);
