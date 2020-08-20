/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "string.h"

/* strcmp variants that return true immediately on identical pointers.
 * The caller is responsible for not passing NULL pointers or pointers
 * into invalid memory blocks, which would cause memory errors in the
 * standard library functions. */
#define RG_STRCMP(a, b) (((a) == (b)) ? 0 : strcmp((a), (b)))
#define RG_STRCASECMP(a, b) (((a) == (b)) ? 0 : strcasecmp((a), (b)))

