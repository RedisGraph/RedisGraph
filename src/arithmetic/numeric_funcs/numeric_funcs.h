/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../../value.h"

void Register_NumericFuncs();

SIValue AR_TOFLOAT(SIValue *argv, int argc, void *private_data);

SIValue AR_TOINTEGER(SIValue *argv, int argc, void *private_data);
