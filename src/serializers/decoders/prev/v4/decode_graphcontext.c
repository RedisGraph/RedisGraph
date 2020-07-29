/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "decode_v4.h"

/* Deserialize unified schema */
static void _RdbLoadAttributeKeys(RedisModuleIO *rdb, GraphContext *gc) {
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
	if(attrCount == 0) return;

	// Heap-allocate the attribute buffer to allow dynamic resizing
	uint buflen = 256;
	char *attr_buf = rm_malloc(buflen * sizeof(char));

	for(uint64_t i = 0; i < attrCount; i++) {
		// Load attribute string from RDB file.
		char *rm_attr = RedisModule_LoadStringBuffer(rdb, &len);

		if(len >= buflen) {
			// Extend the attribute buffer length when encountering strings that would exceed it.
			buflen = len + 1;
			attr_buf = rm_realloc(attr_buf, buflen);
		}

		// The RDB string is not null-terminated, so we need to work on a safe copy.
		memcpy(attr_buf, rm_attr, len);
		attr_buf[len] = 0;

		// Free and skip the RDB string if it's already been mapped
		// (this logic is only necessary if the node and edge schemas are separate)
		GraphContext_FindOrAddAttribute(gc, attr_buf);
		RedisModule_Free(rm_attr);
	}
	rm_free(attr_buf);
}

GraphContext *RdbLoadGraphContext_v4(RedisModuleIO *rdb) {
	/* Format:
	 * graph name
	 * #node schemas
	 * attribute mapping (in encver 4), or unified node schema
	 * node schema X #node schemas
	 * #relation schemas
	 * filler bytes (in encver 4), or unified relation schema
	 * relation schema X #relation schemas
	 * graph object
	 * #indices
	 * (index label, index property) X #indices
	 */

	char *graph_name = RedisModule_LoadStringBuffer(rdb, NULL);
	GraphContext *gc = GraphContext_New(graph_name, GRAPH_DEFAULT_NODE_CAP, GRAPH_DEFAULT_EDGE_CAP);
	RedisModule_Free(graph_name);
	// #Node schemas
	uint32_t schema_count = RedisModule_LoadUnsigned(rdb);

	// Load the full attribute mapping (or the attributes from
	// the unified node schema, if encoding version is < 4)
	_RdbLoadAttributeKeys(rdb, gc);

	// Load each node schema
	gc->node_schemas = array_ensure_cap(gc->node_schemas, schema_count);
	for(uint32_t i = 0; i < schema_count; i ++) {
		gc->node_schemas = array_append(gc->node_schemas, RdbLoadSchema_v4(rdb));
		Graph_AddLabel(gc->g);
	}

	// #Edge schemas
	schema_count = RedisModule_LoadUnsigned(rdb);

	// If encoding version is < 4, load the attributes from the
	// unified edge schema, otherwise skip filler bytes.
	_RdbLoadAttributeKeys(rdb, gc);

	// Load each edge schema
	gc->relation_schemas = array_ensure_cap(gc->relation_schemas, schema_count);
	for(uint32_t i = 0; i < schema_count; i ++) {
		array_append(gc->relation_schemas, RdbLoadSchema_v4(rdb));
		Graph_AddRelationType(gc->g);
	}

	// Graph object.
	RdbLoadGraph_v4(rdb, gc);

	// #Indices
	// (index label, index property) X #indices
	uint32_t index_count = RedisModule_LoadUnsigned(rdb);
	for(uint32_t i = 0; i < index_count; i ++) {
		RdbLoadIndex_v4(rdb, gc);
	}

	/* Build indices
	 * only node schemas have indices. */
	schema_count = GraphContext_SchemaCount(gc, SCHEMA_NODE);
	for(unsigned short i = 0; i < schema_count; i++) {
		Schema *s = GraphContext_GetSchemaByID(gc, i, SCHEMA_NODE);
		assert(s);
		Index *idx = Schema_GetIndex(s, NULL, IDX_EXACT_MATCH);
		if(idx) Index_Construct(idx);
	}

	return gc;
}

