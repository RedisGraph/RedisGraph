/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef XXHASH_QUERY_HASH_H
#define XXHASH_QUERY_HASH_H
#include"../result_set_cache_includes.h"

/**
 * @brief  Hash a query using XXHASH into a 8 byte buffer
 * @param  *query: Query - charectes array
 * @param  queryLength: Query length
 * @param  *buffer: Output buffer to contains to hashed query, buffer should be intialized as:
 *          char buffer[HASH_KEY_LENGTH]
 * @retval None
 */
void hashQuery(const char *query, size_t queryLength, char *buffer);

#endif