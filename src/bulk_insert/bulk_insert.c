/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "bulk_insert.h"
#include "../schema/schema.h"
#include "../util/rmalloc.h"
#include <errno.h>
#include <assert.h>

// The first byte of each property in the binary stream
// is used to indicate the type of the subsequent SIValue
typedef enum {
	BI_NULL,
	BI_BOOL,
	BI_DOUBLE,
	BI_STRING,
	BI_LONG
} TYPE;

// Read the header of a data stream to parse its property keys and update schemas.
static Attribute_ID *_BulkInsert_ReadHeader(GraphContext *gc, SchemaType t,
											const char *data, size_t *data_idx,
											int *label_id, unsigned int *prop_count) {
	/* Binary header format:
	 * - entity name : null-terminated C string
	 * - property count : 4-byte unsigned integer
	 * [0..property_count] : null-terminated C string
	 */
	// First sequence is entity name
	const char *name = data + *data_idx;
	*data_idx += strlen(name) + 1;
	Schema *schema = GraphContext_GetSchema(gc, name, t);
	if(schema == NULL) schema = GraphContext_AddSchema(gc, name, t);
	*label_id = schema->id;

	// Next 4 bytes are property count
	*prop_count = *(unsigned int *)&data[*data_idx];
	*data_idx += sizeof(unsigned int);

	if(*prop_count == 0) return NULL;
	Attribute_ID *prop_indicies = malloc(*prop_count * sizeof(Attribute_ID));

	// The rest of the line is [char *prop_key] * prop_count
	for(unsigned int j = 0; j < *prop_count; j ++) {
		char *prop_key = (char *)data + *data_idx;
		*data_idx += strlen(prop_key) + 1;

		// Add properties to schemas
		prop_indicies[j] = GraphContext_FindOrAddAttribute(gc, prop_key);
	}

	return prop_indicies;
}

// Read an SIValue from the data stream and update the index appropriately
static inline SIValue _BulkInsert_ReadProperty(const char *data, size_t *data_idx) {
	/* Binary property format:
	 * - property type : 1-byte integer corresponding to TYPE enum
	 * - Nothing if type is NULL
	 * - 1-byte true/false if type is boolean
	 * - 8-byte double if type is numeric
	 * - Null-terminated C string if type is string
	 */
	SIValue v;
	TYPE t = data[*data_idx];
	*data_idx += 1;
	if(t == BI_NULL) {
		v = SI_NullVal();
	} else if(t == BI_BOOL) {
		bool b = data[*data_idx];
		*data_idx += 1;
		v = SI_BoolVal(b);
	} else if(t == BI_DOUBLE) {
		double d = *(double *)&data[*data_idx];
		*data_idx += sizeof(double);
		v = SI_DoubleVal(d);
	} else if(t == BI_LONG) {
		int64_t d = *(int64_t *)&data[*data_idx];
		*data_idx += sizeof(int64_t);
		v = SI_LongVal(d);
	} else if(t == BI_STRING) {
		const char *s = data + *data_idx;
		*data_idx += strlen(s) + 1;
		// The string itself will be cloned when added to the GraphEntity properties.
		v = SI_ConstStringVal((char *)s);
	} else {
		assert(0);
	}
	return v;
}

int _BulkInsert_ProcessNodeFile(RedisModuleCtx *ctx, GraphContext *gc, const char *data,
								size_t data_len) {
	size_t data_idx = 0;

	int label_id;
	unsigned int prop_count;
	Attribute_ID *prop_indicies = _BulkInsert_ReadHeader(gc, SCHEMA_NODE, data, &data_idx, &label_id,
														 &prop_count);

	while(data_idx < data_len) {
		Node n;
		Graph_CreateNode(gc->g, label_id, &n);
		for(unsigned int i = 0; i < prop_count; i++) {
			SIValue value = _BulkInsert_ReadProperty(data, &data_idx);
			// Cypher does not support NULL as a property value.
			// If we encounter one here, simply skip it.
			if(SI_TYPE(value) == T_NULL) continue;
			GraphEntity_AddProperty((GraphEntity *)&n, prop_indicies[i], value);
		}
	}

	free(prop_indicies);
	return BULK_OK;
}

int _BulkInsert_ProcessRelationFile(RedisModuleCtx *ctx, GraphContext *gc, const char *data,
									size_t data_len) {
	size_t data_idx = 0;

	int reltype_id;
	unsigned int prop_count;
	// Read property keys from header and update schema
	Attribute_ID *prop_indicies = _BulkInsert_ReadHeader(gc, SCHEMA_EDGE, data, &data_idx, &reltype_id,
														 &prop_count);
	NodeID src;
	NodeID dest;

	while(data_idx < data_len) {
		Edge e;
		// Next 8 bytes are source ID
		src = *(NodeID *)&data[data_idx];
		data_idx += sizeof(NodeID);
		// Next 8 bytes are destination ID
		dest = *(NodeID *)&data[data_idx];
		data_idx += sizeof(NodeID);

		Graph_ConnectNodes(gc->g, src, dest, reltype_id, &e);

		if(prop_count == 0) continue;

		// Process and add relation properties
		for(unsigned int i = 0; i < prop_count; i ++) {
			SIValue value = _BulkInsert_ReadProperty(data, &data_idx);
			// Cypher does not support NULL as a property value.
			// If we encounter one here, simply skip it.
			if(SI_TYPE(value) == T_NULL) continue;
			GraphEntity_AddProperty((GraphEntity *)&e, prop_indicies[i], value);
		}
	}

	free(prop_indicies);
	return BULK_OK;
}

int _BulkInsert_InsertNodes(RedisModuleCtx *ctx, GraphContext *gc, int token_count,
							RedisModuleString ***argv, int *argc) {
	int rc;
	for(int i = 0; i < token_count; i ++) {
		size_t len;
		// Retrieve a pointer to the next binary stream and record its length
		const char *data = RedisModule_StringPtrLen(**argv, &len);
		*argv += 1;
		*argc -= 1;
		rc = _BulkInsert_ProcessNodeFile(ctx, gc, data, len);
		assert(rc == BULK_OK);
	}
	return BULK_OK;
}

int _BulkInsert_Insert_Edges(RedisModuleCtx *ctx, GraphContext *gc, int token_count,
							 RedisModuleString ***argv, int *argc) {
	int rc;
	for(int i = 0; i < token_count; i ++) {
		size_t len;
		// Retrieve a pointer to the next binary stream and record its length
		const char *data = RedisModule_StringPtrLen(**argv, &len);
		*argv += 1;
		*argc -= 1;
		rc = _BulkInsert_ProcessRelationFile(ctx, gc, data, len);
		assert(rc == BULK_OK);
	}
	return BULK_OK;
}

int BulkInsert(RedisModuleCtx *ctx, GraphContext *gc, RedisModuleString **argv, int argc) {

	if(argc < 2) {
		RedisModule_ReplyWithError(ctx, "Bulk insert format error, failed to parse bulk insert sections.");
		return BULK_FAIL;
	}

	// Read the number of node tokens
	long long node_token_count;
	long long relation_token_count;
	if(RedisModule_StringToLongLong(*argv++, &node_token_count)  != REDISMODULE_OK) {
		RedisModule_ReplyWithError(ctx, "Error parsing number of node descriptor tokens.");
		return BULK_FAIL;
	}

	if(RedisModule_StringToLongLong(*argv++, &relation_token_count)  != REDISMODULE_OK) {
		RedisModule_ReplyWithError(ctx, "Error parsing number of relation descriptor tokens.");
		return BULK_FAIL;
	}
	argc -= 2;

	if(node_token_count > 0) {
		int rc = _BulkInsert_InsertNodes(ctx, gc, node_token_count, &argv, &argc);
		if(rc != BULK_OK) {
			return BULK_FAIL;
		} else if(argc == 0) {
			return BULK_OK;
		}
	}

	if(relation_token_count > 0) {
		int rc = _BulkInsert_Insert_Edges(ctx, gc, relation_token_count, &argv, &argc);
		if(rc != BULK_OK) {
			return BULK_FAIL;
		} else if(argc == 0) {
			return BULK_OK;
		}
	}

	assert(argc == 0);

	return BULK_OK;
}

