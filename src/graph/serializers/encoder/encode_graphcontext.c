/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "encode_graphcontext.h"
#include "encode_graph.h"
#include "encode_schema.h"

static void _RdbSaveAttributeKeys(RedisModuleIO *rdb, GraphContext *gc) {
    /* Format:
     * #attribute keys
     * attribute keys
    */

    uint count = GraphContext_AttributeCount(gc);
    RedisModule_SaveUnsigned(rdb, count);
    for (uint i = 0; i < count; i ++) {
        char *key = gc->string_mapping[i];
        RedisModule_SaveStringBuffer(rdb, key, strlen(key));
    }
}

void RdbSaveGraphContext(RedisModuleIO *rdb, void *value) {
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

    GraphContext *gc = value;

    // Lock.
    Graph_AcquireReadLock(gc->g);

    // Graph name.
    RedisModule_SaveStringBuffer(rdb, gc->graph_name, strlen(gc->graph_name) + 1);

    // Serialize all attribute keys
    _RdbSaveAttributeKeys(rdb, gc);

    // #Node schemas.
    unsigned short schema_count = GraphContext_SchemaCount(gc, SCHEMA_NODE);
    RedisModule_SaveUnsigned(rdb, schema_count);

    // Name of label X #node schemas.
    for(int i = 0; i < schema_count; i++) {
        Schema *s = gc->node_schemas[i];
        RdbSaveSchema(rdb, s);
    }

    // #Relation schemas.
    unsigned short relation_count = GraphContext_SchemaCount(gc, SCHEMA_EDGE);
    RedisModule_SaveUnsigned(rdb, relation_count);

    // Name of label X #relation schemas.
    for(unsigned short i = 0; i < relation_count; i++) {
        Schema *s = gc->relation_schemas[i];
        RdbSaveSchema(rdb, s);
    }

    // Serialize graph object
    RdbSaveGraph(rdb, gc);

    // Unlock.
    Graph_ReleaseLock(gc->g);
}
