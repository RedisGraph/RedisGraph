/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include <stdbool.h>
#include <sys/types.h>

typedef struct {
	uint major;
	uint minor;
	uint patch;
} Redis_Version;

// Returns a Version struct with redis version;
Redis_Version RG_GetRedisVersion();

// Checks if the current redis server version is compliant (grater or equal) to a given version.
bool Redis_Version_GreaterOrEqual(uint major, uint minor, uint patch);
