/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "set.h"

set *Set_New(void) {
	return raxNew();
}

bool Set_Contains(set *s, SIValue v) {
	unsigned long long const hash = SIValue_HashCode(v);
	return (raxFind(s, (unsigned char *)&hash, sizeof(hash)) != raxNotFound);
}

/* Adds v to set. */
bool Set_Add(set *s, SIValue v) {
	unsigned long long const hash = SIValue_HashCode(v);
	return raxTryInsert(s, (unsigned char *)&hash, sizeof(hash), NULL, NULL);
}

/* Removes v from set. */
void Set_Remove(set *s, SIValue v) {
	unsigned long long const hash = SIValue_HashCode(v);
	raxRemove(s, (unsigned char *)&hash, sizeof(hash), NULL);
}

/* Return number of elements in set. */
uint64_t Set_Size(set *s) {
	return raxSize(s);
}

/* Free set. */
void Set_Free(set *s) {
	raxFree(s);
}

