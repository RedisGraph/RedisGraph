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

/* binary header format:
 * - entity name : null-terminated C string
 * - property count : 4-byte unsigned integer
 * [0..property_count] : null-terminated C string
 */

// read the label strings from a header, update schemas, and retrieve the label IDs
static int *_BulkInsert_ReadHeaderLabels(GraphContext *gc, SchemaType t,
										 const char *data, size_t *data_idx) {
	ASSERT(gc != NULL);
	ASSERT(data != NULL);
	ASSERT(data_idx != NULL);
	// first sequence is entity label
	const char *labels = data + *data_idx;
	int labels_len = strlen(labels);
	*data_idx += labels_len + 1;

	// Array of all label IDs
	int *label_ids = array_new(int, 1);
	// Stack variable to contain a single label
	char label[labels_len];

	while(true) {
		// Look for a colon delimiting another label
		char *found = strchr(labels, ':');
		if(found) {
			ASSERT(t == SCHEMA_NODE); // Only nodes can have multiple labels
			// This entity file describes multiple labels, copy the current one
			size_t len = found - labels;
			memcpy(label, labels, len);
			label[len] = '\0';
			// Update the labels pointer for the next seek
			labels += len + 1;
		} else {
			// Reached the last (or only) label; copy it
			size_t len = strlen(labels);
			memcpy(label, labels, len + 1);
		}

		// Try to retrieve the label's schema
		Schema *schema = GraphContext_GetSchema(gc, label, t);
		// Create the schema if it does not already exist
		if(schema == NULL) schema = GraphContext_AddSchema(gc, label, t);
		// Store the label ID
		label_ids = array_append(label_ids, schema->id);

		// Break if we've exhausted all labels
		if(!found) break;
	}

	return label_ids;
}

// read the property keys from a header
static Attribute_ID *_BulkInsert_ReadHeaderProperties(GraphContext *gc, SchemaType t,
													  const char *data, size_t *data_idx,
													  uint *prop_count) {
	ASSERT(gc != NULL);
	ASSERT(data != NULL);
	ASSERT(data_idx != NULL);
	ASSERT(prop_count != NULL);

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
static SIValue _BulkInsert_ReadProperty(const char *data, size_t *data_idx) {
	/* Binary property format:
	 * - property type : 1-byte integer corresponding to TYPE enum
	 * - Nothing if type is NULL
	 * - 1-byte true/false if type is boolean
	 * - 8-byte double if type is double
	 * - 8-byte integer if type is integer
	 * - Null-terminated C string if type is string
	 * - 8-byte array length followed by N values if type is array
	 */

	// possible property values
	bool       b;
	double     d;
	int64_t    i;
	int64_t    len;
	const char *s;

	SIValue v = SI_NullVal();
	TYPE t = data[*data_idx];
	*data_idx += 1;

	switch(t) {
		case BI_NULL:
			v = SI_NullVal();
			break;

		case BI_BOOL:
			b = data[*data_idx];
			*data_idx += 1;
			v = SI_BoolVal(b);
			break;

		case BI_DOUBLE:
			d = *(double *)&data[*data_idx];
			*data_idx += sizeof(double);
			v = SI_DoubleVal(d);
			break;

		case BI_LONG:
			i = *(int64_t *)&data[*data_idx];
			*data_idx += sizeof(int64_t);
			v = SI_LongVal(i);
			break;

		case BI_STRING:
			s = data + *data_idx;
			*data_idx += strlen(s) + 1;
			// The string itself will be cloned when added to the GraphEntity properties.
			v = SI_ConstStringVal((char *)s);
			break;

		case BI_ARRAY:
			// The first 8 bytes of a received array will be the array length.
			len = *(int64_t *)&data[*data_idx];
			*data_idx += sizeof(int64_t);
			v = SIArray_New(len);
			for(uint i = 0; i < len; i ++) {
				// Convert every element and add to array.
				SIArray_Append(&v, _BulkInsert_ReadProperty(data, data_idx));
			}
			break;

		default:
			ASSERT(false);
			break;
	}

	return v;
}

static int _BulkInsert_ProcessFile(GraphContext *gc, const char *data,
								   size_t data_len, SchemaType type) {

	uint prop_count;
	size_t data_idx = 0;

	// read the CSV file header labels and update all schemas
	int *label_ids = _BulkInsert_ReadHeaderLabels(gc, type, data, &data_idx);
	uint label_count = array_len(label_ids);
	// read the CSV header properties and collect their indices
	Attribute_ID *prop_indices = _BulkInsert_ReadHeaderProperties(gc, type, data,
																  &data_idx, &prop_count);

	while(data_idx < data_len) {
		Node n;
		Edge e;
		GraphEntity *ge;
		if(type == SCHEMA_NODE) {
			Graph_CreateNode(gc->g, &n, label_ids, label_count);
			ge = (GraphEntity *)&n;
		} else if(type == SCHEMA_EDGE) {
			// next 8 bytes are source ID
			NodeID src = *(NodeID *)&data[data_idx];
			data_idx += sizeof(NodeID);
			// next 8 bytes are destination ID
			NodeID dest = *(NodeID *)&data[data_idx];
			data_idx += sizeof(NodeID);

			Graph_ConnectNodes(gc->g, src, dest, label_ids[0], &e);
			ge = (GraphEntity *)&e;
		} else {
			ASSERT(false);
		}

		// process entity attributes
		for(uint i = 0; i < prop_count; i++) {
			SIValue value = _BulkInsert_ReadProperty(data, &data_idx);
			// skip invalid attribute values
			if(!(SI_TYPE(value) & SI_VALID_PROPERTY_VALUE)) continue;
			GraphEntity_AddProperty(ge, prop_indices[i], value);
		}
	}

	array_free(label_ids);
	if(prop_indices) rm_free(prop_indices);
	return BULK_OK;
}

static int _BulkInsert_ProcessTokens(GraphContext *gc, int token_count,
									 RedisModuleString **argv, SchemaType type) {
	for(int i = 0; i < token_count; i ++) {
		size_t len;
		// retrieve a pointer to the next binary stream and record its length
		const char *data = RedisModule_StringPtrLen(argv[i], &len);
		int rc = _BulkInsert_ProcessFile(gc, data, len, type);
		UNUSED(rc);
		ASSERT(rc == BULK_OK);
	}

	return BULK_OK;
}

int BulkInsert(RedisModuleCtx *ctx, GraphContext *gc, RedisModuleString **argv,
			   int argc, uint node_count, uint edge_count) {

	ASSERT(ctx != NULL);
	ASSERT(gc != NULL);
	ASSERT(argv != NULL);

	if(argc < 2) {
		RedisModule_ReplyWithError(ctx, "Bulk insert format error, \
				failed to parse bulk insert sections.");
		return BULK_FAIL;
	}

	Graph *g = gc->g;
	int res = BULK_OK;

	// lock graph under write lock
	// allocate space for new nodes and edges
	Graph_AcquireWriteLock(g);
	Graph_AllocateNodes(g, node_count);
	Graph_AllocateEdges(g, edge_count);

	// read the number of node tokens
	long long node_token_count;
	long long relation_token_count;

	if(RedisModule_StringToLongLong(*argv++, &node_token_count)  != REDISMODULE_OK) {
		RedisModule_ReplyWithError(ctx, "Error parsing number of node \
				descriptor tokens.");
		res = BULK_FAIL;
		goto cleanup;
	}

	// read the number of relation tokens
	if(RedisModule_StringToLongLong(*argv++, &relation_token_count)  != REDISMODULE_OK) {
		RedisModule_ReplyWithError(ctx, "Error parsing number of relation \
				descriptor tokens.");
		res = BULK_FAIL;
		goto cleanup;
	}

	argc -= 2;

	if(node_token_count > 0) {
		ASSERT(argc >= node_token_count);
		// process all node files
		if(_BulkInsert_ProcessTokens(gc, node_token_count, argv,
									 SCHEMA_NODE) != BULK_OK) {
			res = BULK_FAIL;
			goto cleanup;
		}
		argv += node_token_count;
		argc -= node_token_count;
	}

	if(relation_token_count > 0) {
		ASSERT(argc >= relation_token_count);
		// Process all relationship files
		if(_BulkInsert_ProcessTokens(gc, relation_token_count, argv,
									 SCHEMA_EDGE) != BULK_OK) {
			res = BULK_FAIL;
			goto cleanup;
		}
		argv += relation_token_count;
		argc -= relation_token_count;
	}

	ASSERT(argc == 0);

cleanup:
	Graph_ReleaseLock(g);
	return res;
}

