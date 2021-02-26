/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "bulk_insert.h"
#include "RG.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include "../schema/schema.h"
#include "../datatypes/array.h"
#include "../execution_plan/ops/shared/create_functions.h"

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

// Container struct for nodes and edges that are constructed and committed to the graph.
typedef struct {
	GraphContext *gc;
	Node *nodes;
	PendingProperties *node_props;
	Edge *edges;
	PendingProperties *edge_props;
} PendingInserts;

// Read the header of a data stream to parse its property keys and update schemas.
static Attribute_ID *_BulkInsert_ReadHeader(GraphContext *gc, SchemaType t,
											const char *data, size_t *data_idx,
											int *label_id, uint *prop_count) {
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
	*prop_count = *(uint *)&data[*data_idx];
	*data_idx += sizeof(unsigned int);

	if(*prop_count == 0) return NULL;
	Attribute_ID *prop_indices = rm_malloc(*prop_count * sizeof(Attribute_ID));

	// The rest of the line is [char *prop_key] * prop_count
	for(uint j = 0; j < *prop_count; j ++) {
		char *prop_key = (char *)data + *data_idx;
		*data_idx += strlen(prop_key) + 1;

		// Add properties to schemas
		prop_indices[j] = GraphContext_FindOrAddAttribute(gc, prop_key);
	}

	return prop_indices;
}

// Read an SIValue from the data stream and update the index appropriately
static inline SIValue _BulkInsert_ReadProperty(const char *data, size_t *data_idx) {
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

static int _BulkInsert_ProcessFile(PendingInserts *bulk_ctx, const char *data, size_t data_len,
								   Attribute_ID **label_props, uint *entities_created, SchemaType type) {
	size_t data_idx = 0;

	int label_id;
	uint prop_count;
	// Read the CSV file header and commit all labels and properties it introduces.
	Graph_AcquireWriteLock(bulk_ctx->gc->g);
	Attribute_ID *prop_indices = _BulkInsert_ReadHeader(bulk_ctx->gc, type, data, &data_idx,
														&label_id, &prop_count);
	Graph_ReleaseLock(bulk_ctx->gc->g);
	*label_props = prop_indices;
	while(data_idx < data_len) {
		if(type == SCHEMA_NODE) {
			Node n = GE_NEW_LABELED_NODE(NULL, label_id);
			bulk_ctx->nodes[*entities_created] = n;
		} else if(type == SCHEMA_EDGE) {
			Edge e;
			e.relationID = label_id;
			// Next 8 bytes are source ID
			e.srcNodeID = *(NodeID *)&data[data_idx];
			data_idx += sizeof(NodeID);
			// Next 8 bytes are destination ID
			e.destNodeID = *(NodeID *)&data[data_idx];
			data_idx += sizeof(NodeID);
			bulk_ctx->edges[*entities_created] = e;
		} else {
			ASSERT(false);
		}
		PendingProperties props;
		props.values = rm_malloc(prop_count * sizeof(SIValue));
		props.attr_keys = prop_indices;
		props.property_count = prop_count;
		for(uint i = 0; i < prop_count; i++) {
			props.values[i] = _BulkInsert_ReadProperty(data, &data_idx);
		}
		if(type == SCHEMA_NODE) {
			bulk_ctx->node_props[*entities_created] = props;
		} else if(type == SCHEMA_EDGE) {
			bulk_ctx->edge_props[*entities_created] = props;
		} else {
			ASSERT(false);
		}
		(*entities_created) ++;
	}

	return BULK_OK;
}

static int _BulkInsert_ProcessTokens(PendingInserts *bulk_ctx, int token_count,
									 RedisModuleString ***argv, int *argc, Attribute_ID **prop_ids, SchemaType type) {
	uint entities_created = 0;
	for(int i = 0; i < token_count; i ++) {
		size_t len;
		// Retrieve a pointer to the next binary stream and record its length
		const char *data = RedisModule_StringPtrLen(**argv, &len);
		*argv += 1;
		*argc -= 1;
		int rc = _BulkInsert_ProcessFile(bulk_ctx, data, len, &prop_ids[i], &entities_created, type);
		UNUSED(rc);
		ASSERT(rc == BULK_OK);
	}
	return BULK_OK;
}

static void _BulkInsert_Commit(PendingInserts *bulk_ctx, uint node_count, uint edge_count) {
	Graph *g = bulk_ctx->gc->g;
	// Number of entities already created
	size_t initial_node_count = Graph_NodeCount(g);

	// Disable matrix synchronization for bulk insert operation
	Graph_SetMatrixPolicy(g, RESIZE_TO_CAPACITY);

	// Allocate or extend datablocks to accommodate all incoming entities
	Graph_AllocateNodes(g, node_count + initial_node_count);

	// Commit all nodes to the graph
	for(uint i = 0; i < node_count; i ++) {
		Node n = bulk_ctx->nodes[i];
		Graph_CreateNode(bulk_ctx->gc->g, n.labelID, &n);
		PendingProperties props = bulk_ctx->node_props[i];
		for(uint j = 0; j < props.property_count; j ++) {
			SIValue prop = props.values[j];
			// Cypher does not support NULL as a property value.
			// If we encounter one here, simply skip it.
			if(SIValue_IsNull(prop)) continue;
			GraphEntity_AddProperty((GraphEntity *)&n, props.attr_keys[j], props.values[j]);
		}
		rm_free(props.values);
	}

	// Commit all edges to the graph
	for(uint i = 0; i < edge_count; i ++) {
		Edge e = bulk_ctx->edges[i];
		Graph_ConnectNodes(bulk_ctx->gc->g, e.srcNodeID, e.destNodeID, e.relationID, &e);
		PendingProperties props = bulk_ctx->edge_props[i];
		for(uint j = 0; j < props.property_count; j ++) {
			SIValue prop = props.values[j];
			// Cypher does not support NULL as a property value.
			// If we encounter one here, simply skip it.
			if(SIValue_IsNull(prop)) continue;
			GraphEntity_AddProperty((GraphEntity *)&e, props.attr_keys[j], props.values[j]);
		}
		rm_free(props.values);
	}
}

int BulkInsert(RedisModuleCtx *ctx, GraphContext *gc, RedisModuleString **argv, int argc,
			   uint node_count, uint edge_count) {

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

	// Read the number of relation tokens
	if(RedisModule_StringToLongLong(*argv++, &relation_token_count)  != REDISMODULE_OK) {
		RedisModule_ReplyWithError(ctx, "Error parsing number of relation descriptor tokens.");
		return BULK_FAIL;
	}
	argc -= 2;

	PendingInserts bulk_ctx = { .gc = gc };

	// Stack-allocate an array to contain the attribute key IDs of each label file.
	Attribute_ID *props_per_label[node_token_count];
	if(node_token_count > 0) {
		bulk_ctx.nodes = rm_malloc(node_count * sizeof(Node));
		bulk_ctx.node_props = rm_malloc(node_count * sizeof(PendingProperties));
		// Process all node files
		int rc = _BulkInsert_ProcessTokens(&bulk_ctx, node_token_count, &argv, &argc,
										   props_per_label, SCHEMA_NODE);
		if(rc != BULK_OK) return BULK_FAIL;
	}

	// Stack-allocate an array to contain the attribute key IDs of each relationship type file.
	Attribute_ID *props_per_type[relation_token_count];
	if(relation_token_count > 0) {
		bulk_ctx.edges = rm_malloc(edge_count * sizeof(Edge));
		bulk_ctx.edge_props = rm_malloc(edge_count * sizeof(PendingProperties));
		// Process all relationship files
		int rc = _BulkInsert_ProcessTokens(&bulk_ctx, relation_token_count, &argv, &argc,
										   props_per_type, SCHEMA_EDGE);
		if(rc != BULK_OK) return BULK_FAIL;
	}

	ASSERT(argc == 0);

	// Lock the graph for writing
	RedisModule_ThreadSafeContextLock(ctx);
	Graph_AcquireWriteLock(gc->g);

	// Commit all accumulated changes
	_BulkInsert_Commit(&bulk_ctx, node_count, edge_count);

	// Release the lock
	Graph_ReleaseLock(gc->g);
	RedisModule_ThreadSafeContextUnlock(ctx);

	// Free accumulated data
	for(uint i = 0; i < node_token_count; i ++) rm_free(props_per_label[i]);
	for(uint i = 0; i < relation_token_count; i ++) rm_free(props_per_type[i]);
	rm_free(bulk_ctx.nodes);
	rm_free(bulk_ctx.node_props);
	rm_free(bulk_ctx.edges);
	rm_free(bulk_ctx.edge_props);

	return BULK_OK;
}

