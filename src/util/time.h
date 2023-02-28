/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include <stdint.h>

// Returns the milliseconds elapsed since the UNIX epoch.
uint64_t get_unix_timestamp_milliseconds(void);
