/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include <stddef.h>
#include <stdbool.h>
#include "../../deps/rax/rax.h"

// Returns true if 'b' is a subset of 'a'.
bool raxIsSubset(rax *a, rax *b);

// Duplicates a rax, performing a shallow copy of the original's values.
rax *raxClone(rax *orig);
