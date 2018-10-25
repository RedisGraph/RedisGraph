/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <assert.h>
#include "../graph.h"
#include "serialize_graph.h"
#include "../../GraphBLASExt/tuples_iter.h"

void _RdbLoadMatrix(RedisModuleIO *rdb, GrB_Matrix m) {
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

void _RdbLoadMatrices(RedisModuleIO *rdb, Graph *g) {
    /* Format:
     * adjacency matrix
     * #relation matrices
     *      #entries N
     *      (row index,column index) X N
     * #label matrices
     *      #entries N
     *      (row index,column index) X N
     */

    _RdbLoadMatrix(rdb, g->adjacency_matrix);

    uint64_t relationMatricesCount = RedisModule_LoadUnsigned(rdb);
    for(int i = 0; i < relationMatricesCount; i++) {
        Graph_AddRelationType(g);
        GrB_Matrix m = Graph_GetRelation(g, i);
        _RdbLoadMatrix(rdb, m);
    }

    uint64_t labelMatricesCount = RedisModule_LoadUnsigned(rdb);
    for(int i = 0; i < labelMatricesCount; i++) {
        Graph_AddLabel(g);
        GrB_Matrix m = Graph_GetLabel(g, i);
        _RdbLoadMatrix(rdb, m);
    }
}

SIValue _RdbLoadSIValue(RedisModuleIO *rdb) {
    /* Format:
     * SIType
     * Value */
    SIType t = RedisModule_LoadUnsigned(rdb);
    if(t & SI_NUMERIC) {
        return SI_DoubleVal(RedisModule_LoadDouble(rdb));
    } else if (t == T_BOOL) {
        return SI_BoolVal(RedisModule_LoadUnsigned(rdb));
    } else {
        return SI_StringVal(RedisModule_LoadStringBuffer(rdb, NULL));
    }
}

void _RdbLoadEntity(RedisModuleIO *rdb, GraphEntity *e) {
    /* Format:
     * #properties N
     * (name, SIValue) X N
    */
    uint64_t propCount = RedisModule_LoadUnsigned(rdb);
    size_t propNameLen;
    char *propName[propCount];
    SIValue propValue[propCount];

    for(int i = 0; i < propCount; i++) {
        propName[i] = RedisModule_LoadStringBuffer(rdb, &propNameLen);
        propValue[i] = _RdbLoadSIValue(rdb);
    }

    if(propCount) GraphEntity_Add_Properties(e, propCount, propName, propValue);
}

void _RdbLoadNodes(RedisModuleIO *rdb, Graph *g) {
    /* Format:
     * #nodes
    */
    uint64_t nodeCount = RedisModule_LoadUnsigned(rdb);
    if(nodeCount == 0) return;

    DataBlockIterator *iter;
    Graph_CreateNodes(g, nodeCount, NULL, &iter);

    Node *n;
    while((n = (Node*)DataBlockIterator_Next(iter))) {
        _RdbLoadEntity(rdb, (GraphEntity*)n);
    }

    DataBlockIterator_Free(iter);
}

void _RdbLoadEdges(RedisModuleIO *rdb, Graph *g) {
    /* Format:
     * #edges (N)
     * {
     *  source node ID
     *  destination node ID
     *  relation type
     * } X N
    */

    uint64_t edgeCount = RedisModule_LoadUnsigned(rdb);
    if(edgeCount == 0) return;

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
        _RdbLoadEntity(rdb, (GraphEntity*)e);
    }

    DataBlockIterator_Free(iter);
}

void _RdbSaveSIValue(RedisModuleIO *rdb, const SIValue *v) {
    /* Format:
     * SIType
     * Value */
    RedisModule_SaveUnsigned(rdb, v->type);
    if(v->type & SI_NUMERIC) {
        RedisModule_SaveDouble(rdb, v->doubleval);
    } else if (v->type == T_BOOL) {
        RedisModule_SaveUnsigned(rdb, v->boolval);
    } else if (v->type == T_STRING) {
        RedisModule_SaveStringBuffer(rdb, v->stringval, strlen(v->stringval) + 1);
    } else {
        assert(0 && "Attempted to serialize value of invalid type.");
    }
}

void _RdbSaveEntity(RedisModuleIO *rdb, const GraphEntity *e) {
    /* Format:
     * #properties N
     * (name, SIValue) X N */

    RedisModule_SaveUnsigned(rdb, e->prop_count);

    for(int i = 0; i < e->prop_count; i++) {
        EntityProperty prop = e->properties[i];
        RedisModule_SaveStringBuffer(rdb, prop.name, strlen(prop.name) + 1);
        _RdbSaveSIValue(rdb, &prop.value);
    }
}

void _RdbSaveNode(RedisModuleIO *rdb, const Node *n) {
    _RdbSaveEntity(rdb, (GraphEntity*)n);
}

void _RdbSaveNodes(RedisModuleIO *rdb, const Graph *g) {
    /* Format:
     * #nodes
     * nodes */

    RedisModule_SaveUnsigned(rdb, Graph_NodeCount(g));
    DataBlockIterator *iter = Graph_ScanNodes(g);
    Node *n;
    while((n = (Node*)DataBlockIterator_Next(iter))) {
        _RdbSaveNode(rdb, n);
    }

    DataBlockIterator_Free(iter);
}

void _RdbSaveEdge(RedisModuleIO *rdb, const Edge *e) {
    _RdbSaveEntity(rdb, (GraphEntity*)e);
}

void _RdbSaveEdges(RedisModuleIO *rdb, const Graph *g) {
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
        _RdbSaveEdge(rdb, e);
    }

    DataBlockIterator_Free(iter);
}

void _RdbSaveMatrix(RedisModuleIO *rdb, GrB_Matrix m) {
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

void _RdbSaveMatrices(RedisModuleIO *rdb, Graph *g) {
    /* Format:
     * adjacency matrix
     * #relation matrices
     * relation matrices
     * #label matrices
     * label matrices
     * */

    _RdbSaveMatrix(rdb, g->adjacency_matrix);

    RedisModule_SaveUnsigned(rdb, g->relation_count);
    for(int i = 0; i < g->relation_count; i++) {
        GrB_Matrix m = Graph_GetRelation(g, i);
        _RdbSaveMatrix(rdb, m);
    }

    RedisModule_SaveUnsigned(rdb, g->label_count);
    for(int i = 0; i < g->label_count; i++) {
        GrB_Matrix m = Graph_GetLabel(g, i);
        _RdbSaveMatrix(rdb, m);
    }
}

void RdbSaveGraph(RedisModuleIO *rdb, void *value) {
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

    Graph *g = (Graph *)value;

    // Dump nodes.
    _RdbSaveNodes(rdb, g);

    // Dump relation and label matrices.
    _RdbSaveMatrices(rdb, g);

    // Dump edges.
    _RdbSaveEdges(rdb, g);
}

void *RdbLoadGraph(RedisModuleIO *rdb) {
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
    Graph *g = Graph_New(GRAPH_DEFAULT_NODE_CAP);

    // Load nodes.
    _RdbLoadNodes(rdb, g);

    // Load matrices.
    _RdbLoadMatrices(rdb, g);

    // Load edges.
    _RdbLoadEdges(rdb, g);

    // Flush all pending changes to graphs.
    GrB_wait();

    return g;
}

