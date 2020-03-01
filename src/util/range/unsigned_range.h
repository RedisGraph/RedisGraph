/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	uint64_t min;
	uint64_t max;
	bool include_min;
	bool include_max;
	bool valid;
} UnsignedRange;

UnsignedRange *UnsignedRange_New(void);
UnsignedRange *UnsignedRange_Clone(const UnsignedRange *range);
bool UnsignedRange_IsValid(const UnsignedRange *range);
bool UnsignedRange_ContainsValue(const UnsignedRange *range, uint64_t v);
void UnsignedRange_TightenRange(UnsignedRange *range, int op, uint64_t v);
void UnsignedRange_ToString(const UnsignedRange *range);
void UnsignedRange_Free(UnsignedRange *range);
