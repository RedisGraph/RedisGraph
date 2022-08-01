/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

/* TODO: not sure if this is the right place for this file
 * another possibility would be ./src/util */
#pragma once

#include <stddef.h>
#include "rax.h"
#include "value.h"

typedef rax set;

/* Create a new set. */
set *Set_New(void);

/* Check to see if v is in set. */
bool Set_Contains(set *s, SIValue v);

/* Adds v to set. */
bool Set_Add(set *s, SIValue v);

/* Removes v from set. */
void Set_Remove(set *s, SIValue v);

/* Return number of elements in set. */
uint64_t Set_Size(set *s);

/* Free set. */
void Set_Free(set *s);
