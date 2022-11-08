/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <time.h>
#include "temporal_value.h"

int64_t TemporalValue_NewTimestamp() {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return 1000 * ts.tv_sec + ts.tv_nsec / 1000000;;
}
