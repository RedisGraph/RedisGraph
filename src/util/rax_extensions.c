#include "rax_extensions.h"

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

