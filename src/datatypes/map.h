/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

/* A map is a set of key/value pairs 
 * where key is a string and value can be anyone of the following types:
 * map, array, node, edge, path, date, string, bool, numeric and NULL 
 *
 * this implimantaion of map, uses SIValue for the stored values
 * and keeps the keys sorted for quick lookup. 
 *
 * the underline structure of map is an array of key/value pairs
 * [ (key/value), (key/value), ... (key/value) ] */

#include "../value.h"

typedef struct Pair {
	SIValue key;  // key associated with value
	SIValue val;  // value stored under key
} Pair;

typedef Pair* Map;

// create a new map
SIValue Map_New
(
	uint capacity     // map initial capacity
);

// clones map
SIValue Map_Clone
(
	SIValue map  // map to clone
);

// adds key/value to map
void Map_Add
(
	SIValue *map,  // map to add element to
	SIValue key,   // key under which value is added
	SIValue value  // value to add under key
);

// removes key from map
void Map_Remove
(
	SIValue map,  // map to remove key from
	SIValue key   // key to remove
);

// retrieves value under key, map[key]
// return true and set 'value' if key is in map
// otherwise return false
bool Map_Get
(
	SIValue map,    // map to get value from
	SIValue key,    // key to lookup value
	SIValue *value  // [output] value to retrieve
);

// checks if 'key' is in map
bool Map_Contains
(
	SIValue map,  // map to query
	SIValue key   // key to look-up
);

// return number of keys in map
uint Map_KeyCount
(
	SIValue map  // map to count number of keys in
);

// return all keys in map
// caller should free returned array with array_free
SIValue *Map_Keys
(
	SIValue map  // map to extract keys from
);

// populate 'buf' with string representation of map
void Map_ToString
(
	SIValue map,          // map to get string representation from
	char **buf,           // buffer to populate
	size_t *bufferLen,    // size of buffer
	size_t *bytesWritten  // length of string
);

// free map
void Map_Free
(
	SIValue map  // map to free
);

