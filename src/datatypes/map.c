/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "map.h"
#include "../util/arr.h"
#include "../util/strcmp.h"
#include "../util/rmalloc.h"

static Pair Pair_New(SIValue key, SIValue val) {
	ASSERT(SI_TYPE(key) & T_STRING);
	// TODO: should we also clone the key?
	Pair pair = { .key = key, .val = SI_CloneValue(val) };
	return pair;
}

static void Pair_Free(Pair p) {
	SIValue_Free(p.key);
	SIValue_Free(p.val);
}

static int Map_KeyIdx(SIValue map, SIValue key) {
	ASSERT(SI_TYPE(map) & T_MAP);
	ASSERT(SI_TYPE(key) & T_STRING);

	Map m = map.map;
	uint n = array_len(m);

	// search for key in map
	for(uint i = 0; i < n; i++) {
		Pair pair = m[i];
		if(RG_STRCMP(pair.key.stringval, key.stringval) == 0) {
			return i;
		}
	}

	// key not in map
	return -1;
}

// create a new map
SIValue Map_New
(
	uint capacity
)
{
	SIValue map;
	map.map = array_new(Pair, capacity);
	map.type = T_MAP;
	map.allocation = M_SELF;
	return map;
}

// clone map
SIValue Map_Clone
(
	SIValue map  // map to clone
)
{
	ASSERT(SI_TYPE(map) & T_MAP);

	uint key_count = Map_KeyCount(map);
	SIValue clone = Map_New(key_count);

	for(uint i = 0; i < key_count; i++) {
		Pair p = map.map[i];
		Map_Add(&clone, p.key, p.val);
	}

	return clone;
}

// adds key/value to map
void Map_Add
(
	SIValue *map,
	SIValue key,
	SIValue value
)
{
	ASSERT(SI_TYPE(*map) & T_MAP);
	ASSERT(SI_TYPE(key) & T_STRING);

	// remove key if already existed
	Map_Remove(*map, key);

	// create a new pair
	Pair pair = Pair_New(key, value);

	// add pair to the end of map
	map->map = array_append(map->map, pair);
}

// removes key from map
void Map_Remove
(
	SIValue map,
	SIValue key
)
{
	ASSERT(SI_TYPE(map) & T_MAP);
	ASSERT(SI_TYPE(key) & T_STRING);

	Map m = map.map;

	// search for key in map
	int idx = Map_KeyIdx(map, key);
	
	// key missing from map
	if(idx == -1) return;
	
	// override removed key with last pair
	uint last_idx = array_len(m) - 1;
	m[idx] = m[last_idx];

	// replace pair at 'idx' with last pair
	array_pop(m);
}

// retrieves value under key, map[key]
// sets 'value' to NULL if key isn't in map
bool Map_Get
(
	SIValue map,
	SIValue key,
	SIValue *value
)
{
	ASSERT(SI_TYPE(map) & T_MAP);
	ASSERT(SI_TYPE(key) & T_STRING);
	ASSERT(value != NULL);

	int idx = Map_KeyIdx(map, key);

	// key isn't in map, set 'value' to NULL and return
	if(idx == -1) {
		*value = SI_NullVal();
		return false;
	} else {
		*value = map.map[idx].val;
		return true;
	}
}

// checks if 'key' is in map
bool Map_Contains
(
	SIValue map,
	SIValue key
)
{
	ASSERT(SI_TYPE(map) & T_MAP);
	ASSERT(SI_TYPE(key) & T_STRING);

	return (Map_KeyIdx(map, key) != -1);
}

uint Map_KeyCount
(
	SIValue map
)
{
	ASSERT(SI_TYPE(map) & T_MAP);
	return array_len(map.map);
}

SIValue *Map_Keys
(
	SIValue map
)
{
	ASSERT(SI_TYPE(map) & T_MAP);
	
	uint key_count = Map_KeyCount(map);
	SIValue *keys = array_new(SIValue, key_count);
	
	for(uint i = 0; i < key_count; i++) {
		Pair p = map.map[i];
		keys = array_append(keys, p.key);
	}

	return keys;
}

int Map_Compare
(
	SIValue mapA,
	SIValue mapB,
	int *disjointOrNull
)
{
	int   order        =  0;
	Map   A            =  mapA.map;
	Map   B            =  mapB.map;
	uint  A_key_count  =  Map_KeyCount(mapA);
	uint  B_key_count  =  Map_KeyCount(mapB);

	if(A_key_count != B_key_count) {
		if(A_key_count > B_key_count) return 1;
		else return -1;
	}

	// TODO: sort maps

	uint len = MIN(A_key_count, B_key_count);

	// Check empty list.
	// notEqual holds the first false (result != 0)
	// comparison result between two values from the same type,
	// which are not equal.

	// go over the common range for both maps
	for(uint i = 0; i < len; i++) {
		Pair Ap = A[i];
		Pair Bp = B[i];

		// compare keys
		order = strcmp(Ap.key.stringval, Bp.key.stringval);
		if(order != 0) return order;

		// same key, compare values
		order = SIValue_Compare(Ap.val, Bp.val, disjointOrNull);
		if(*disjointOrNull == COMPARED_NULL || *disjointOrNull == DISJOINT)
			return 0;

		if(order != 0) return order;
	}

	// maps are equal
	return 0;
}

/* This method referenced by Java ArrayList.hashCode() method, which takes
 * into account the hasing of nested values.*/
XXH64_hash_t Map_HashCode
(
	SIValue map
)
{
	// TODO: sort on key
	// {a:1, b:1} and {b:1, a:1} should have the same hash value

	SIType t = SI_TYPE(map);
	XXH64_hash_t hashCode = XXH64(&t, sizeof(t), 0);
	uint key_count = Map_KeyCount(map);

	for(uint i = 0; i < key_count; i++) {
		Pair p = map.map[i];
		hashCode = 31 * hashCode + SIValue_HashCode(p.key);
		hashCode = 31 * hashCode + SIValue_HashCode(p.val);
	}

	return hashCode;
}

void Map_ToString
(
	SIValue map,          // map to get string representation from
	char **buf,           // buffer to populate
	size_t *bufferLen,    // size of buffer
	size_t *bytesWritten  // length of string
)
{
	ASSERT(SI_TYPE(map) & T_MAP);
	ASSERT(buf != NULL);
	ASSERT(bufferLen != NULL);
	ASSERT(bytesWritten != NULL);

	// resize buffer if buffer length is less than 64
	if(*bufferLen - *bytesWritten < 64) {
		*bufferLen += 64;
		*buf = rm_realloc(*buf, sizeof(char) * *bufferLen);
	}

	uint key_count = Map_KeyCount(map);

	// "{" marks the beginning of a map
	*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "{");

	for(uint i = 0; i < key_count; i ++) {
		Pair p = map.map[i];
		// write the next key/value pair
		SIValue_ToString(p.key, buf, bufferLen, bytesWritten);
		*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, ": ");
		SIValue_ToString(p.val, buf, bufferLen, bytesWritten);
		// if this is not the last element, add ", "
		if(i != key_count - 1) *bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, ", ");
	}

	// make sure there's enough space for "}"
	if(*bufferLen - *bytesWritten < 2) {
		*bufferLen += 2;
		*buf = rm_realloc(*buf, sizeof(char) * *bufferLen);
	}

	// "}" marks the end of a map
	*bytesWritten += snprintf(*buf + *bytesWritten, *bufferLen, "}");
}

// free map
void Map_Free
(
	SIValue map
)
{
	ASSERT(SI_TYPE(map) & T_MAP);

	uint l = Map_KeyCount(map);

	// free stored pairs
	for(uint i = 0; i < l; i++) {
		Pair p = map.map[i];
		Pair_Free(p);	
	}

	array_free(map.map);
}

