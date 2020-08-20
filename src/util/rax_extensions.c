/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "rax_extensions.h"
#include "arr.h"

bool raxIsSubset(rax *a, rax *b) {
	raxIterator it;
	raxStart(&it, b);
	raxSeek(&it, "^", NULL, 0);

	while(raxNext(&it)) { // For each key in b
		// Break if key is not present in a
		if(raxFind(a, it.key, it.key_len) == raxNotFound) break;
	}
	bool is_subset = raxEOF(&it); // True if iterator was depleted.

	raxStop(&it);
	return is_subset;
}

rax *raxClone(rax *orig) {
	rax *rax = raxNew();

	raxIterator it;
	raxStart(&it, orig);
	raxSeek(&it, "^", NULL, 0);

	// For each key in the original, duplicate the key and its value in the clone.
	while(raxNext(&it)) {
		raxInsert(rax, it.key, it.key_len, it.data, NULL);
	}

	raxStop(&it);
	return rax;
}

void **raxValues(rax *rax) {
	// Instantiate an array to hold all of the values in the rax.
	void **values = array_new(void *, raxSize(rax));
	raxIterator it;
	raxStart(&it, rax);
	// Iterate over all keys in the rax.
	raxSeek(&it, "^", NULL, 0);
	while(raxNext(&it)) {
		// Copy the value associated with the key into the array.
		values = array_append(values, it.data);
	}
	raxStop(&it);

	return values;
}

unsigned char **raxKeys(rax *rax) {
	// Instantiate an array to hold all of the keys in the rax.
	unsigned char **keys = array_new(unsigned char *, raxSize(rax));
	raxIterator it;
	raxStart(&it, rax);
	// Iterate over all keys in the rax.
	raxSeek(&it, "^", NULL, 0);
	while(raxNext(&it)) {
		// Copy the key into the array.
		keys = array_append(keys, (unsigned char *)rm_strndup((const char *)it.key, (int)it.key_len));
	}
	raxStop(&it);
	return keys;
}
