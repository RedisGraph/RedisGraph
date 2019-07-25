/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "prev_decode_schema.h"

#include "../../../../util/arr.h"
#include "../../../../util/rmalloc.h"

/* Deserialize unified schema */
void PrevRdbLoadAttributeKeys(RedisModuleIO *rdb, GraphContext *gc) {
	/* Format:
	 * id // (fake)
	 * name // (fake)
	 * #attribute keys
	 * attribute keys
	 */

	RedisModule_LoadUnsigned(rdb);
	char *name = RedisModule_LoadStringBuffer(rdb, NULL);
	RedisModule_Free(name);

	size_t len = 0;
	uint64_t attrCount = RedisModule_LoadUnsigned(rdb);

	for(uint64_t i = 0; i < attrCount; i++) {
		// Load attribute string from RDB file.
		char *attr = RedisModule_LoadStringBuffer(rdb, &len);

		// Free and skip the RDB string if it's already been mapped
		// (this logic is only necessary if the node and edge schemas are separate)
		if(TrieMap_Find(gc->attributes, attr, len) != TRIEMAP_NOTFOUND) {
			// Free the RDB string
			RedisModule_Free(attr);
			continue;
		}

		// Add attribute string if it hasn't been encountered before.
		Attribute_ID *pAttribute_id = NULL;
		pAttribute_id = rm_malloc(sizeof(Attribute_ID));
		*pAttribute_id = GraphContext_AttributeCount(gc);

		// Update the string->ID triemap
		TrieMap_Add(gc->attributes, attr, len, pAttribute_id, TrieMap_DONT_CARE_REPLACE);
		// Update the ID->string array
		// The RDB string was not null-terminated, so we need to make an updated copy
		char *rm_attr = rm_malloc((len + 1) * sizeof(char));
		memcpy(rm_attr, attr, len);
		rm_attr[len] = '\0';
		gc->string_mapping = array_append(gc->string_mapping, rm_attr);
		RedisModule_Free(attr);
	}
}

Schema *PrevRdbLoadSchema(RedisModuleIO *rdb, SchemaType type) {
	/* Format:
	 * id
	 * name
	 * #attributes
	 * attributes
	 */

	int id = RedisModule_LoadUnsigned(rdb);
	char *name = RedisModule_LoadStringBuffer(rdb, NULL);
	Schema *s = Schema_New(name, id);

	uint64_t attrCount = RedisModule_LoadUnsigned(rdb);

	// Only doing this for backwards compatibility; we're not keeping these strings
	for(uint64_t i = 0; i < attrCount; i++) {
		// Load attribute string from RDB file.
		char *attr = RedisModule_LoadStringBuffer(rdb, NULL);
		RedisModule_Free(attr);
	}

	return s;
}
