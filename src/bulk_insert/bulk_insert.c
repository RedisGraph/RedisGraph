/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "bulk_insert.h"
#include "RG.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include "../schema/schema.h"
#include "../datatypes/array.h"

// The first byte of each property in the binary stream
// is used to indicate the type of the subsequent SIValue
typedef enum {
	BI_NULL = 0,
	BI_BOOL = 1,
	BI_DOUBLE = 2,
	BI_STRING = 3,
	BI_LONG = 4,
	BI_ARRAY = 5,
} TYPE;

// Read the header of a data stream to parse its property keys and update schemas.
static Attribute_ID *_BulkInsert_ReadHeader(GraphContext *gc, SchemaType t,
											const char *data, size_t *data_idx,
											int *label_id, uint *prop_count) {
	/* binary header format:
	 * - entity name : null-terminated C string
	 * - property count : 4-byte unsigned integer
	 * [0..property_count] : null-terminated C string
	 */

	// first sequence is entity name
	const char *name = data + *data_idx;
	*data_idx += strlen(name) + 1;
	Schema *schema = GraphContext_GetSchema(gc, name, t);
	if(schema == NULL) schema = GraphContext_AddSchema(gc, name, t);
	*label_id = schema->id;

	// next 4 bytes are property count
	*prop_count = *(uint *)&data[*data_idx];
	*data_idx += sizeof(unsigned int);

	if(*prop_count == 0) return NULL;
	Attribute_ID *prop_indices = rm_malloc(*prop_count * sizeof(Attribute_ID));

	// the rest of the line is [char *prop_key] * prop_count
	for(uint j = 0; j < *prop_count; j ++) {
		char *prop_key = (char *)data + *data_idx;
		*data_idx += strlen(prop_key) + 1;

		// add properties to schemas
		prop_indices[j] = GraphContext_FindOrAddAttribute(gc, prop_key);
	}

	return prop_indices;
}

// Read an SIValue from the data stream and update the index appropriately
static inline SIValue _BulkInsert_ReadProperty(const char *data,
											   size_t *data_idx) {
	/* Binary property format:
	 * - property type : 1-byte integer corresponding to TYPE enum
	 * - Nothing if type is NULL
	 * - 1-byte true/false if type is boolean
	 * - 8-byte double if type is double
	 * - 8-byte integer if type is integer
	 * - Null-terminated C string if type is string
	 * - 8-byte array length followed by N values if type is array
	 */

	SIValue v = SI_NullVal();
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
	} else if(t == BI_ARRAY) {
		// The first 8 bytes of a received array will be the array length.
		int64_t len = *(int64_t *)&data[*data_idx];
		*data_idx += sizeof(int64_t);
		v = SIArray_New(len);
		for(uint i = 0; i < len; i ++) {
			// Convert every element and add to array.
			SIArray_Append(&v, _BulkInsert_ReadProperty(data, data_idx));
		}
	} else {
		ASSERT(false);
	}
	return v;
}

static int _BulkInsert_ProcessFile(GraphContext *gc, const char *data,
								   size_t data_len, SchemaType type) {

	int label_id;
	uint prop_count;
	size_t data_idx = 0;

	// read the CSV file header
	// and commit all labels and properties it introduces
	Graph_AcquireWriteLock(gc->g);
	Attribute_ID *prop_indices = _BulkInsert_ReadHeader(gc, type, data, &data_idx,
														&label_id, &prop_count);
	while(data_idx < data_len) {
		Node n;
		Edge e;
		GraphEntity *ge;
		if(type == SCHEMA_NODE) {
			Graph_CreateNode(gc->g, label_id, &n);
			ge = (GraphEntity *)&n;
		} else if(type == SCHEMA_EDGE) {
			// Next 8 bytes are source ID
			NodeID src = *(NodeID *)&data[data_idx];
			data_idx += sizeof(NodeID);
			// Next 8 bytes are destination ID
			NodeID dest = *(NodeID *)&data[data_idx];
			data_idx += sizeof(NodeID);

			Graph_ConnectNodes(gc->g, src, dest, label_id, &e);
			ge = (GraphEntity *)&e;
		} else {
			ASSERT(false);
		}

		for(uint i = 0; i < prop_count; i++) {
			SIValue value = _BulkInsert_ReadProperty(data, &data_idx);
			// Cypher does not support NULL as a property value.
			// If we encounter one here, simply skip it.
			if(SI_TYPE(value) == T_NULL) continue;
			GraphEntity_AddProperty(ge, prop_indices[i], value);
		}
	}

	// release the lock
	Graph_ReleaseLock(gc->g);
	rm_free(prop_indices);
	return BULK_OK;
}

static int _BulkInsert_ProcessTokens(GraphContext *gc, int token_count,
									 RedisModuleString ***argv, int *argc, SchemaType type) {
	uint entities_created = 0;
	for(int i = 0; i < token_count; i ++) {
		size_t len;
		// retrieve a pointer to the next binary stream and record its length
		const char *data = RedisModule_StringPtrLen(**argv, &len);
		*argv += 1;
		*argc -= 1;
		int rc = _BulkInsert_ProcessFile(gc, data, len, type);
		UNUSED(rc);
		ASSERT(rc == BULK_OK);
	}
	return BULK_OK;
}

int BulkInsert(RedisModuleCtx *ctx, GraphContext *gc, RedisModuleString **argv, int argc,
			   uint node_count, uint edge_count) {

	if(argc < 2) {
		RedisModule_ReplyWithError(ctx, "Bulk insert format error, failed to parse bulk insert sections.");
		return BULK_FAIL;
	}

	// allocate space for new nodes and edges
	Graph_AcquireWriteLock(gc->g);
	DataBlock_Accommodate(gc->g->nodes, node_count);
	DataBlock_Accommodate(gc->g->edges, edge_count);
	Graph_ReleaseLock(gc->g);

	// read the number of node tokens
	long long node_token_count;
	long long relation_token_count;

	if(RedisModule_StringToLongLong(*argv++, &node_token_count)  != REDISMODULE_OK) {
		RedisModule_ReplyWithError(ctx, "Error parsing number of node descriptor tokens.");
		return BULK_FAIL;
	}

	// read the number of relation tokens
	if(RedisModule_StringToLongLong(*argv++, &relation_token_count)  != REDISMODULE_OK) {
		RedisModule_ReplyWithError(ctx, "Error parsing number of relation descriptor tokens.");
		return BULK_FAIL;
	}

	argc -= 2;

	if(node_token_count > 0) {
		// process all node files
		int rc = _BulkInsert_ProcessTokens(gc, node_token_count, &argv,
										   &argc, SCHEMA_NODE);
		if(rc != BULK_OK) return BULK_FAIL;
	}

	if(relation_token_count > 0) {
		// Process all relationship files
		int rc = _BulkInsert_ProcessTokens(gc, relation_token_count,
										   &argv, &argc, SCHEMA_EDGE);
		if(rc != BULK_OK) return BULK_FAIL;
	}

	ASSERT(argc == 0);

	return BULK_OK;
}

