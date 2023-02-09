/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "time.h"
#include <sys/time.h>

uint64_t get_unix_timestamp_milliseconds(void) {
	struct timeval tv = {};

	gettimeofday(&tv, NULL);

	const uint64_t milliseconds_since_epoch =
		(uint64_t)(tv.tv_sec) * 1000 +
		(uint64_t)(tv.tv_usec) / 1000;

	return milliseconds_since_epoch;
}
