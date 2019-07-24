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

typedef enum {
	TIME = 1,
	LOCAL_TIME = 2,
	DATE = 4,
	DATE_TIME = 8,
	LOCAL_DATE_TIME = 16,
	DURATION = 32
} RG_TemporalType;

typedef struct {
	time_t seconds;             // holds nano seconds from epoch
	uint32_t nano;              // holds nano seconds delta
	unsigned int type : 6;      // holds type
	unsigned int timeZone : 26; // bitmap to hold temporal type and timezone id
} RG_TemporalValue;

// new temporal values
/* Create new date according to system clock */
RG_TemporalValue RG_Date_New();

/* Create new time according to system clock, in UTC 00:00 offset as defualt */
RG_TemporalValue RG_Time_New();

/* Create new time according to system local time */
RG_TemporalValue RG_LocalTime_New();

/* Create new date time according to system clock, in UTC 00:00 offset as defualt */
RG_TemporalValue RG_DateTime_New();

/* Create new date time according to system local time */
RG_TemporalValue RG_LocalDateTime_New();

// testing & future feature - build new temporal values from given timestamp value
/* Create new date from given timestamp */
RG_TemporalValue RG_Date_New_FromTimeSpec(struct timespec ts);

/* Create new time from given timestamp UTC 00:00 offset as defualt*/
RG_TemporalValue RG_Time_New_FromTimeSpec(struct timespec ts);

/* Create new local time from given timestamp*/
RG_TemporalValue RG_LocalTime_New_FromTimeSpec(struct timespec ts);

/* Create new datetime from given timestamp UTC 00:00 offset as defualt */
RG_TemporalValue RG_DateTime_New_FromTimeSpec(struct timespec ts);

/* Create new local datetime from given timestamp */
RG_TemporalValue RG_LocalDateTime_New_FromTimeSpec(struct timespec ts);

// build from string

/* Create new date from a given cypher string */
RG_TemporalValue RG_Date_New_FromString(const char *str);

/* Create new time from a given cypher string */
RG_TemporalValue RG_Time_New_FromString(const char *str);

/* Create new local time from a given cypher string */
RG_TemporalValue RG_LocalTime_New_FromString(const char *str);

/* Create new datetime from a given cypher string */
RG_TemporalValue RG_DateTime_New_FromString(const char *str);

/* Create new local datetime from a new cypher string */
RG_TemporalValue RG_LocalDateTime_New_FromString(const char *str);

/**
  * @brief  Returns the year of supported temporal values
  * @note   TIME, LOCAL_TIME and DURATION values are not supported
  * @param  temporalValue: relevant temporal value
  * @retval year in the range [-999,999,999:999,999,999] for relevant values. INT64_MIN otherwise
  */
int64_t RG_TemporalValue_GetYear(RG_TemporalValue timeValue);

/**
  * @brief  returns the quarter for supported values
  * @note   TIME, LOCAL_TIME and DURATION values are not supported
  * @param  temporalValue: relevant temporal value
  * @retval quater in the range [1:4] for relevant values. INT64_MIN otherwise
  */
int64_t RG_TemporalValue_GetQuarter(RG_TemporalValue timeValue);

/**
  * @brief  returns the month for supported values
  * @note   TIME, LOCAL_TIME and DURATION values are not supported
  * @param  temporalValue: relevant temporal value
  * @retval month in the range [1:12] for relevant values. INT64_MIN otherwise
  */
int64_t RG_TemporalValue_GetMonth(RG_TemporalValue temporalValue);

/**
  * @brief  returns the week for supported values
  * @note   TIME, LOCAL_TIME and DURATION values are not supported
  * @param  temporalValue: relevant temporal value
  * @retval year in the range [1:53] for relevant values. INT64_MIN otherwise
  */
int64_t RG_TemporalValue_GetWeek(RG_TemporalValue temporalValue);

/**
  * @brief  returns the year of the week for supported values
  * @note   TIME, LOCAL_TIME and DURATION values are not supported
  * @param  temporalValue: relevant temporal value
  * @retval year in the range [-999,999,999:999,999,999] for relevant values. INT64_MIN otherwise
  */
int64_t RG_TemporalValue_GetWeekYear(RG_TemporalValue temporalValue);

/**
  * @brief  returns the day in the quarter for supported values
  * @note   TIME, LOCAL_TIME and DURATION values are not supported
  * @param  temporalValue: relevant temporal value
  * @retval day in the range [1:92] for relevant values. INT64_MIN otherwise
  */
int64_t RG_TemporalValue_GetDayOfQuarter(RG_TemporalValue temporalValue);

/**
  * @brief  returns the day in the month for supported values
  * @note   TIME, LOCAL_TIME and DURATION values are not supported
  * @param  temporalValue: relevant temporal value
  * @retval day in the range [1:31] for relevant values. INT64_MIN otherwise
  */
int64_t RG_TemporalValue_GetDay(RG_TemporalValue temporalValue);

/**
  * @brief  returns the day in the year for supported values
  * @note   TIME, LOCAL_TIME and DURATION values are not supported
  * @param  temporalValue: relevant temporal value
  * @retval day in the range [1:366] for relevant values. INT64_MIN otherwise
  */
int64_t RG_TemporalValue_GetOrdinalDay(RG_TemporalValue temporalValue);

/**
  * @brief  returns the day in the week for supported values
  * @note   TIME, LOCAL_TIME and DURATION values are not supported
  * @param  temporalValue: relevant temporal value
  * @retval day in the range [1:7] for relevant values. INT64_MIN otherwise
  */
int64_t RG_TemporalValue_GetDayOfWeek(RG_TemporalValue temporalValue);

/**
  * @brief  returns the hour for supported values
  * @note   DATE and DURATION values are not supported
  * @param  temporalValue: relevant temporal value
  * @retval hour in the range [0:23] for relevant values. INT64_MIN otherwise
  */
int64_t RG_TemporalValue_GetHour(RG_TemporalValue temporalValue);

/**
  * @brief  returns the minute for supported values
  * @note   DATE and DURATION values are not supported
  * @param  temporalValue: relevant temporal value
  * @retval minute in the range [0:59] for relevant values. INT64_MIN otherwise
  */
int64_t RG_TemporalValue_GetMinute(RG_TemporalValue temporalValue);

/**
  * @brief  returns the second for supported values
  * @note   DATE and DURATION values are not supported
  * @param  temporalValue: relevant temporal value
  * @retval second in the range [0:60] for relevant values. INT64_MIN otherwise
  */
int64_t RG_TemporalValue_GetSecond(RG_TemporalValue temporalValue);

/**
  * @brief  returns the millisecond for supported values
  * @note   DATE and DURATION values are not supported
  * @param  temporalValue: relevant temporal value
  * @retval millisecond in the range [0:999] for relevant values. INT64_MIN otherwise
  */
int64_t RG_TemporalValue_GetMillisecond(RG_TemporalValue temporalValue);

/**
  * @brief  returns the microsecond for supported values
  * @note   DATE and DURATION values are not supported
  * @param  temporalValue: relevant temporal value
  * @retval microsecond in the range [0:999,999] for relevant values. INT64_MIN otherwise
  */
int64_t RG_TemporalValue_GetMicrosecond(RG_TemporalValue temporalValue);

/**
  * @brief  returns the nanosecond for supported values
  * @note   DATE and DURATION values are not supported
  * @param  temporalValue: relevant temporal value
  * @retval nanosecond in the range [0:999,999,999] for relevant values. INT64_MIN otherwise
  */
int64_t RG_TemporalValue_GetNanosecond(RG_TemporalValue temporalValue);

// TODO: implement the following:
// const char* RG_TemporalValue_GetTimeZone(RG_TemporalValue temporalValue);
// const char* RG_TemporalValue_GetOffset(RG_TemporalValue temporalValue);
// int64_t RG_TemporalValue_GetOffsetMinuets(RG_TemporalValue temporalValue);
// int64_t RG_TemporalValue_GetOffsetSeconds(RG_TemporalValue temporalValue);
// int64_t RG_TemporalValue_GetEpochMillis(RG_TemporalValue temporalValue);
// int64_t RG_TemporalValue_GetEpochSeconds(RG_TemporalValue temporalValue);

/**
  * @brief  toString function
  * @note
  * @param  temporalValue:
  * @retval a string represantion of the value
  */

const char *RG_TemporalValue_ToString(RG_TemporalValue temporalValue);

/**
  * @brief  compare between two temporal values
  * @note
  * @param  a: temporal value
  * @param  b: temporal value
  * @retval negative value if a < b, 0 if equal, positive if b < a
  */

int RG_TemporalValue_Compare(RG_TemporalValue a, RG_TemporalValue b);
