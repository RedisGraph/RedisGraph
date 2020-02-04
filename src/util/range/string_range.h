/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <stdbool.h>

typedef struct {
    char *min;
    char *max;
    bool include_min;
    bool include_max;
    bool valid;
} StringRange;

StringRange* StringRange_New(void);
bool StringRange_IsValid(const StringRange *range);
bool StringRange_ContainsValue(const StringRange *range, const char *v);
void StringRange_TightenRange(StringRange *range, int op, const char *v);
void StringRange_ToString(const StringRange *range);
void StringRange_Free(StringRange *range);
