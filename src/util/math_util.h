/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

// computes x + y
// returns to true if x+y overflowed
bool safe_add
(
	int x,  // x
	int y,  // y
	int *z  // x+y
);

// computes x * y
// returns to true if x*y overflowed
bool safe_mul
(
	int x,  // x
	int y,  // y
	int *z  // x*y
);

