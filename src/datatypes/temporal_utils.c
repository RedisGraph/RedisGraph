/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "temporal_utils.h"

const int nomralYearQuarterDays[4] = {90, 91, 92, 92};
const int normalYearQuarterDaysAgg[4] = {0, 90, 181, 273};
const int leapYearQuarterDays[4] = {90, 92, 92, 92};
const int leapYearQuarterDaysAgg[4] = {0, 91, 182, 274};

const int normalYearMonthDaysAgg[11] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304};
const int leapYearMonthDaysAgg[11] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305};

bool isLeapYear(int year) {
	if(year % 400 == 0)
		return true;
	if(year % 100 == 0)
		return false;
	if(year % 4 == 0)
		return true;
	return false;
}