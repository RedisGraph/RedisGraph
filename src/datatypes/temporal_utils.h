/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once
#include <stdbool.h>

const int normalYearQuarterDaysAgg[4] = {0, 90, 181, 273};
const int leapYearQuarterDaysAgg[4] = {0, 91, 182, 274};

const int normalYearMonthDaysAgg[11] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304};
const int leapYearMonthDaysAgg[11] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305};

bool isLeapYear(int year);