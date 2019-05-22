/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "xxhash_query_hash.h"
#include "xxhash/xxhash.h"

unsigned long long const hashQuery(const char *query, size_t queryLength)
{
    return  XXH64(query, queryLength, 0);
}


