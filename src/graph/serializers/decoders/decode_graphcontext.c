/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "decode_graphcontext.h"

#include "decode_graph.h"
#include "decode_schema.h"
#include "../../../util/arr.h"
#include "../../../util/rmalloc.h"

/* Thread local storage graph context key. */
extern pthread_key_t _tlsGCKey;

static void _RdbLoadAttributeKeys(RedisModuleIO *rdb, GraphContext *gc) {
    /* Format:
     * #attribute keys
     * attribute keys
    */
   
    uint count = RedisModule_LoadUnsigned(rdb);
    for (uint i = 0; i < count; i ++) {
        char *attr = RedisModule_LoadStringBuffer(rdb, NULL);
        GraphContext_FindOrAddAttribute(gc, attr);
    }
}

GraphContext* RdbLoadGraphContext(RedisModuleIO *rdb) {
    /* Format:
     * graph name
     * attribute keys (unified schema)
     * #node schemas
     * node schema X #node schemas
     * #relation schemas
     * unified relation schema
     * relation schema X #relation schemas
     * graph object
    */

    GraphContext *gc = rm_calloc(1, sizeof(GraphContext));
    // Graph context defaults
    gc->index_count = 0;
    gc->attributes = NewTrieMap();
    gc->string_mapping = array_new(char*, 64);
    gc->g = Graph_New(GRAPH_DEFAULT_NODE_CAP, GRAPH_DEFAULT_EDGE_CAP);

    // _tlsGCKey was created as part of module load.
    pthread_setspecific(_tlsGCKey, gc);

    // Graph name
    gc->graph_name = RedisModule_LoadStringBuffer(rdb, NULL);

    // Attributes, Load the full attribute mapping.
    _RdbLoadAttributeKeys(rdb, gc);

    // #Node schemas
    uint schema_count = RedisModule_LoadUnsigned(rdb);

    // Load each node schema
    gc->node_schemas = array_new(Schema*, schema_count);
    for (uint i = 0; i < schema_count; i ++) {
        array_append(gc->node_schemas, RdbLoadSchema(rdb, SCHEMA_NODE));
        Graph_AddLabel(gc->g);
    }

    // #Edge schemas
    schema_count = RedisModule_LoadUnsigned(rdb);

    // Load each edge schema
    gc->relation_schemas = array_new(Schema*, schema_count);
    for (uint i = 0; i < schema_count; i ++) {
        array_append(gc->relation_schemas, RdbLoadSchema(rdb, SCHEMA_EDGE));
        Graph_AddRelationType(gc->g);
    }

    // Graph object.    
    RdbLoadGraph(rdb, gc);

    // TODO: Populate indicies.

    return gc;
}
