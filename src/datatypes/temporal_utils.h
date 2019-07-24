/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once
#include <stdbool.h>

extern const int nomralYearQuarterDays[4];
extern const int normalYearQuarterDaysAgg[4];
extern const int leapYearQuarterDays[4];
extern const int leapYearQuarterDaysAgg[4];

extern const int normalYearMonthDaysAgg[11];
extern const int leapYearMonthDaysAgg[11];

bool isLeapYear(int year);