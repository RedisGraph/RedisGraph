#pragma once

#include "../value.h"

SIValue Array_New(u_int64_t initialCapacity);

SIValue Array_Append(SIValue siarray, SIValue value);

SIValue Array_Get(SIValue siarray, u_int64_t index);

u_int32_t Array_Length(SIValue siarray);

SIValue Array_Clone(SIValue siarray);

void Array_Free(SIValue siarray);