/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "serialize_schema.h"
#include "../../util/arr.h"

/* Deserialize unified schema */
void RdbLoadAttributeKeys(RedisModuleIO *rdb, GraphContext *gc) {
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

Schema *RdbLoadSchema(RedisModuleIO *rdb, SchemaType type) {
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

void RdbSaveSchema(RedisModuleIO *rdb, Schema *s) {
	/* Format:
	 * id
	 * name
	 * #attributes // TODO unnecessary
	 * attributes  // TODO unnecessary
	 */

	RedisModule_SaveUnsigned(rdb, s->id);
	RedisModule_SaveStringBuffer(rdb, s->name, strlen(s->name) + 1);
	// Don't save any strings
	RedisModule_SaveUnsigned(rdb, 0);
}

/* Serialize dummy values to fill expected space in RDB file.
 * This is primarily required for the second unified schema (edges),
 * as the space for the first is used to encode all attribute keys
 */
void RdbSaveDummySchema(RedisModuleIO *rdb) {
	/* Format:
	 * id
	 * name
	 * #attributes
	 * attributes (nothing, as # is 0)
	 */

	RedisModule_SaveUnsigned(rdb, 0);
	RedisModule_SaveStringBuffer(rdb, "", 1);
	RedisModule_SaveUnsigned(rdb, 0);
}

void RdbSaveAttributeKeys(RedisModuleIO *rdb, GraphContext *gc) {
	/* Format:
	 * id // (fake)
	 * name // (fake)
	 * #attribute keys
	 * attribute keys
	 */

	RedisModule_SaveUnsigned(rdb, 0);
	RedisModule_SaveStringBuffer(rdb, "", 1);
	uint count = GraphContext_AttributeCount(gc);
	RedisModule_SaveUnsigned(rdb, count);
	for(uint i = 0; i < count; i ++) {
		char *key = gc->string_mapping[i];
		RedisModule_SaveStringBuffer(rdb, key, strlen(key));
	}
}

