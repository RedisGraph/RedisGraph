/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <stdbool.h>

typedef struct {
    double min;
    double max;
    bool include_min;
    bool include_max;
    bool valid;
} NumericRange;

NumericRange* NumericRange_New(void);
bool NumericRange_IsValid(const NumericRange *range);
bool NumericRange_ContainsValue(const NumericRange *range, double v);
void NumericRange_TightenRange(NumericRange *range, int op, double v);
void NumericRange_ToString(const NumericRange *range);
void NumericRange_Free(NumericRange *range);
