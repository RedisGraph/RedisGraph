/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <stdbool.h>
#include "temporal_value.h"
#include "../util/rmalloc.h"

int64_t RG_TemporalValue_NewTimestamp() {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return 1000 * ts.tv_sec + ts.tv_nsec / 1000000;;
}
