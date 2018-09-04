/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "graph.h"
#include "graph_type.h"
#include "../arithmetic/tuples_iter.h"

/* Declaration of the type for redis registration. */
RedisModuleType *GraphRedisModuleType;

void _GraphType_LoadMatrix(RedisModuleIO *rdb, GrB_Matrix m) {
    /* Format:     
     *      #entries N
     *      (row index,column index) X N
     */

    GrB_Index i;
    GrB_Index j;
    GrB_Index nvals;
    nvals = RedisModule_LoadUnsigned(rdb);

    while(nvals) {
        i = RedisModule_LoadUnsigned(rdb);
        j = RedisModule_LoadUnsigned(rdb);
        GrB_Matrix_setElement_BOOL(m, true, i, j);
        nvals--;
    }
}

void _GraphType_LoadMatrices(RedisModuleIO *rdb, Graph *g) {
    /* Format:
     * adjacency matrix
     * #relation matrices
     *      #entries N
     *      (row index,column index) X N 
     * #label matrices
     *      #entries N
     *      (row index,column index) X N 
     */

    _GraphType_LoadMatrix(rdb, g->adjacency_matrix);

    uint64_t relationMatricesCount = RedisModule_LoadUnsigned(rdb);
    for(int i = 0; i < relationMatricesCount; i++) {
        int matrixIdx = Graph_AddRelation(g);
        GrB_Matrix m = Graph_GetRelation(g, matrixIdx);
        _GraphType_LoadMatrix(rdb, m);
    }

    uint64_t labelMatricesCount = RedisModule_LoadUnsigned(rdb);
    for(int i = 0; i < labelMatricesCount; i++) {
        int matrixIdx = Graph_AddLabel(g);
        GrB_Matrix m = Graph_GetLabel(g, matrixIdx);
        _GraphType_LoadMatrix(rdb, m);
    }
}

void _GraphType_LoadEntity(RedisModuleIO *rdb, GraphEntity *e) {
    /* Format:
     * #properties N
     * (name, value type, value) X N
    */
    uint64_t propCount = RedisModule_LoadUnsigned(rdb);
    size_t propNameLen;
    char *propName[propCount];
    SIValue propValue[propCount];

    for(int i = 0; i < propCount; i++) {
        propName[i] = RedisModule_LoadStringBuffer(rdb, &propNameLen);
        SIType t = RedisModule_LoadUnsigned(rdb);
        SIValue val;
        if(t & SI_NUMERIC) {
            propValue[i] = SI_DoubleVal(RedisModule_LoadDouble(rdb));
        } else {
            propValue[i] = SI_StringVal(RedisModule_LoadStringBuffer(rdb, NULL));
        }
    }

    if(propCount) GraphEntity_Add_Properties(e, propCount, propName, propValue);
}

void _GraphType_LoadNodes(RedisModuleIO *rdb, Graph *g) {
    /* Format:
     * #nodes
    */
    uint64_t nodeCount = RedisModule_LoadUnsigned(rdb);
    DataBlockIterator *iter;
    Graph_CreateNodes(g, nodeCount, NULL, &iter);

    Node *n;
    while((n = (Node*)DataBlockIterator_Next(iter))) {
        _GraphType_LoadEntity(rdb, (GraphEntity*)n);
    }

    DataBlockIterator_Free(iter);
}

void _GraphType_LoadEdges(RedisModuleIO *rdb, Graph *g) {
    /* Format:
     * #edges (N)
     * {
     *  source node ID
     *  destination node ID
     *  relation type
     * } X N
    */

    uint64_t edgeCount = RedisModule_LoadUnsigned(rdb);
    /* TODO: it might be better to process edges in batches,
     * such that we do not allocate huge chunks of memory. */
    EdgeDesc *connections = malloc(edgeCount * sizeof(EdgeDesc));

    // Construct connections.
    for(int i = 0; i < edgeCount; i++) {
        connections[i].srcId = RedisModule_LoadUnsigned(rdb);
        connections[i].destId = RedisModule_LoadUnsigned(rdb);
        connections[i].relationId = RedisModule_LoadUnsigned(rdb);
    }

    DataBlockIterator *iter;
    Graph_ConnectNodes(g, connections, edgeCount, &iter);
    free(connections);

    Edge *e;
    while((e = (Edge*)DataBlockIterator_Next(iter))) {
        _GraphType_LoadEntity(rdb, (GraphEntity*)e);
    }

    DataBlockIterator_Free(iter);
}

void _GraphType_SaveEntity(RedisModuleIO *rdb, const GraphEntity *e) {
    /* Format:
     * #properties N
     * (name, value type, value) X N */

    RedisModule_SaveUnsigned(rdb, e->prop_count);

    for(int i = 0; i < e->prop_count; i++) {
        EntityProperty prop = e->properties[i];
        RedisModule_SaveStringBuffer(rdb, prop.name, strlen(prop.name) + 1);
        
        RedisModule_SaveUnsigned(rdb, prop.value.type);
        
        if(prop.value.type & SI_NUMERIC) {
            RedisModule_SaveDouble(rdb, prop.value.doubleval);
        } else {
            RedisModule_SaveStringBuffer(rdb, prop.value.stringval, strlen(prop.value.stringval) + 1);
        }        
    }
}

void _GraphType_SaveNode(RedisModuleIO *rdb, const Node *n) {
    _GraphType_SaveEntity(rdb, (GraphEntity*)n);
}

void _GraphType_SaveNodes(RedisModuleIO *rdb, const Graph *g) {
    /* Format:
     * #nodes 
     * nodes */
    
    RedisModule_SaveUnsigned(rdb, Graph_NodeCount(g));
    DataBlockIterator *iter = Graph_ScanNodes(g);
    Node *n;
    while((n = (Node*)DataBlockIterator_Next(iter))) {
        _GraphType_SaveNode(rdb, n);
    }

    DataBlockIterator_Free(iter);
}

void _GraphType_SaveEdge(RedisModuleIO *rdb, const Edge *e) {
    _GraphType_SaveEntity(rdb, (GraphEntity*)e);
}

void _GraphType_SaveEdges(RedisModuleIO *rdb, const Graph *g) {
    /* Format:
     * #edges (N)
     * {
     *  source node ID
     *  destination node ID
     *  relation type
     * } X N
     * edge properties X N */
    
    RedisModule_SaveUnsigned(rdb, Graph_EdgeCount(g));
    DataBlockIterator *iter = Graph_ScanEdges(g);
    Edge *e;
    while((e = (Edge*)DataBlockIterator_Next(iter))) {
        RedisModule_SaveUnsigned(rdb, Edge_GetSrcNodeID(e));
        RedisModule_SaveUnsigned(rdb, Edge_GetDestNodeID(e));
        RedisModule_SaveUnsigned(rdb, Edge_GetRelationID(e));
    }
    DataBlockIterator_Reset(iter);

    while((e = (Edge*)DataBlockIterator_Next(iter))) {
        _GraphType_SaveEdge(rdb, e);
    }

    DataBlockIterator_Free(iter);
}

void _GraphType_SaveMatrix(RedisModuleIO *rdb, GrB_Matrix m) {
    /* Format:
     * #entries N
     * (row index,column index) X N */
    TuplesIter *iter = TuplesIter_new(m);
    
    GrB_Index nvals;
    GrB_Index row;
    GrB_Index col;
    
    GrB_Matrix_nvals(&nvals, m);
    RedisModule_SaveUnsigned(rdb, nvals);

    while(TuplesIter_next(iter, &row, &col) == TuplesIter_OK) {
        RedisModule_SaveUnsigned(rdb, row);
        RedisModule_SaveUnsigned(rdb, col);
    }

    TuplesIter_free(iter);
}

void _GraphType_SaveMatrices(RedisModuleIO *rdb, Graph *g) {
    /* Format: 
     * adjacency matrix
     * #relation matrices
     * relation matrices
     * #label matrices
     * label matrices
     * */
    
    _GraphType_SaveMatrix(rdb, g->adjacency_matrix);

    RedisModule_SaveUnsigned(rdb, g->relation_count);
    for(int i = 0; i < g->relation_count; i++) {
        GrB_Matrix m = Graph_GetRelation(g, i);
        _GraphType_SaveMatrix(rdb, m);
    }

    RedisModule_SaveUnsigned(rdb, g->label_count);
    for(int i = 0; i < g->label_count; i++) {
        GrB_Matrix m = Graph_GetLabel(g, i);
        _GraphType_SaveMatrix(rdb, m);
    }
}

void GraphType_RdbSave(RedisModuleIO *rdb, void *value) {
    /* Format:
     * nodes 
     * relations
     * edges */

    Graph *g = (Graph *)value;

    // Dump nodes.
    _GraphType_SaveNodes(rdb, g);

    // Dump relations.
    _GraphType_SaveMatrices(rdb, g);

    // Dump edges.
    _GraphType_SaveEdges(rdb, g);
}

void *GraphType_RdbLoad(RedisModuleIO *rdb, int encver) {
    /* Format:
     * #nodes
     *      #properties N
     *      (name, value type, value) X N 
     *
     * adjacency matrix
     * #relation matrices
     *      #entries N
     *      (row index,column index) X N 
     * #label matrices
     *      #entries N
     *      (row index,column index) X N 
     */
    
    if (encver > GRAPH_TYPE_ENCODING_VERSION) {
        return NULL;
    }

    Graph *g = Graph_New(GRAPH_DEFAULT_NODE_CAP);

    // Load nodes.
    _GraphType_LoadNodes(rdb, g);

    // Load matrices.
    _GraphType_LoadMatrices(rdb, g);
    
    // Load edges.
    _GraphType_LoadEdges(rdb, g);

    return g;
}

void GraphType_AofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value) {
  // TODO: implement.
}

void GraphType_Free(void *value) {
  Graph *g = value;
  Graph_Free(g);
}

int GraphType_Register(RedisModuleCtx *ctx) {
  RedisModuleTypeMethods tm = {.version = REDISMODULE_TYPE_METHOD_VERSION,
                               .rdb_load = GraphType_RdbLoad,
                               .rdb_save = GraphType_RdbSave,
                               .aof_rewrite = GraphType_AofRewrite,
                               .free = GraphType_Free};
  
  GraphRedisModuleType = RedisModule_CreateDataType(ctx, "graphtype", GRAPH_TYPE_ENCODING_VERSION, &tm);
  if (GraphRedisModuleType == NULL) {
    return REDISMODULE_ERR;
  }
  return REDISMODULE_OK;
}
