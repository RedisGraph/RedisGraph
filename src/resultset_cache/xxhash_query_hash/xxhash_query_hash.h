/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef XXHASH_QUERY_HASH_H
#define XXHASH_QUERY_HASH_H
#include"../result_set_cache_includes.h"

/**
 * @brief  Hash a query using XXHASH into a 64 bit
 * @param  *query: Query - charectes array
 * @param  queryLength: Query length
 * @retval hash value
 */
unsigned long long const hashQuery(const char *query, size_t queryLength);

#endif