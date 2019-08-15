/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "prev_decode_graphcontext.h"
#include "prev_decode_graph.h"
#include "prev_decode_index.h"
#include "prev_decode_schema.h"
#include "../../../../util/arr.h"
#include "../../../../util/rmalloc.h"

extern pthread_key_t _tlsGCKey;    // Thread local storage graph context key.

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
	if(attrCount == 0) return;

	// Heap-allocate the attribute buffer to allow dynamic resizing
	uint buflen = 256;
	char *attr_buf = rm_malloc(buflen * sizeof(char));

	for(uint64_t i = 0; i < attrCount; i++) {
		// Load attribute string from RDB file.
		char *rm_attr = RedisModule_LoadStringBuffer(rdb, &len);

		if(len >= buflen) {
			// Double the attribute buffer length when encountering strings that would exceed it.
			buflen *= 2;
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

GraphContext *PrevRdbLoadGraphContext(RedisModuleIO *rdb) {
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

	GraphContext *gc = rm_calloc(1, sizeof(GraphContext));

	// _tlsGCKey was created as part of module load.
	pthread_setspecific(_tlsGCKey, gc);

	// Graph name.
	gc->graph_name = RedisModule_LoadStringBuffer(rdb, NULL);
	gc->g = Graph_New(GRAPH_DEFAULT_NODE_CAP, GRAPH_DEFAULT_EDGE_CAP);

	// Initialize property mappings.
	gc->attributes = NewTrieMap();
	gc->string_mapping = array_new(char *, 64);

	// #Node schemas
	uint32_t schema_count = RedisModule_LoadUnsigned(rdb);

	// Load the full attribute mapping (or the attributes from
	// the unified node schema, if encoding version is < 4)
	PrevRdbLoadAttributeKeys(rdb, gc);

	// Load each node schema
	gc->node_schemas = array_new(Schema *, schema_count);
	for(uint32_t i = 0; i < schema_count; i ++) {
		array_append(gc->node_schemas, PrevRdbLoadSchema(rdb));
		Graph_AddLabel(gc->g);
	}

	// #Edge schemas
	schema_count = RedisModule_LoadUnsigned(rdb);

	// If encoding version is < 4, load the attributes from the
	// unified edge schema, otherwise skip filler bytes.
	PrevRdbLoadAttributeKeys(rdb, gc);

	// Load each edge schema
	gc->relation_schemas = array_new(Schema *, schema_count);
	for(uint32_t i = 0; i < schema_count; i ++) {
		array_append(gc->relation_schemas, PrevRdbLoadSchema(rdb));
		Graph_AddRelationType(gc->g);
	}

	// Graph object.
	PrevRdbLoadGraph(rdb, gc);

	// #Indices
	// (index label, index property) X #indices
	uint32_t index_count = RedisModule_LoadUnsigned(rdb);
	for(uint32_t i = 0; i < index_count; i ++) {
		PrevRdbLoadIndex(rdb, gc);
	}

	/* Build indices
	 * only node schemas have indices. */
	unsigned short indices_count = GraphContext_SchemaCount(gc, SCHEMA_NODE);
	for(unsigned short i = 0; i < indices_count; i++) {
		Schema *s = GraphContext_GetSchemaByID(gc, i, SCHEMA_NODE);
		assert(s);
		Index *idx = Schema_GetIndex(s, NULL, IDX_EXACT_MATCH);
		if(idx) Index_Construct(idx);
	}

	return gc;
}
