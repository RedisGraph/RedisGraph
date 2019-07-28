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

// new temporal values
/* Create a new timestamp - millis from epoch */
int64_t RG_TemporalValue_NewTimestamp();
