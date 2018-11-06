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
#include "../../util/arr.h"
#include "../../util/qsort.h"

// Find elem position within array, if elem isn't present in array
// its insertion position is returned.
static uint64_t _binarySearch(uint64_t *array, NodeID id) {
    uint32_t deletedIndiciesCount = array_len(array);
    uint32_t left = 0;
    uint32_t right = deletedIndiciesCount-1;
    uint32_t pos;
    while(left <= right) {
        pos = (right + left)/2;
        if(id < array[pos]) right = pos-1;
        else left = pos+1;
    }
    return pos;
}

static NodeID _updatedID(uint64_t *array, NodeID id) {
    uint32_t itemCount = array_len(array);
    if(itemCount == 0) return id;
    else if(id > array[itemCount-1]) return id - itemCount;
    else if(id < array[0]) return id;
    else return id - _binarySearch(array, id);
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
     * (name, value type, value) X N
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
     *      ID
     *      #labels M
     *      (labels) X M
     *      #properties N
     *      (name, value type, value) X N
    */
    uint64_t nodeCount = RedisModule_LoadUnsigned(rdb);
    if(nodeCount == 0) return;

    for(uint64_t i = 0; i < nodeCount; i++) {
        Node n;
        // * ID
        NodeID id = RedisModule_LoadUnsigned(rdb);

        // * #labels M
        // * (labels) X M
        uint64_t nodeLabelCount = RedisModule_LoadUnsigned(rdb);
        // Ignore nodeLabelCount.
        uint64_t l = RedisModule_LoadUnsigned(rdb);
        Graph_CreateNode(g, l, &n);

        _RdbLoadEntity(rdb, (GraphEntity*)&n);
    }
}

void _RdbLoadEdges(RedisModuleIO *rdb, Graph *g) {
    /* Format:
     * #edges (N)
     * {
     *  edge ID, currently not in use.
     *  source node ID
     *  destination node ID
     *  relation type
     * } X N
     * edge properties X N */

    uint64_t edgeCount = RedisModule_LoadUnsigned(rdb);
    if(edgeCount == 0) return;

    // Construct connections.
    for(int i = 0; i < edgeCount; i++) {
        Edge e;
        EdgeID edgeId = RedisModule_LoadUnsigned(rdb);
        NodeID srcId = RedisModule_LoadUnsigned(rdb);
        NodeID destId = RedisModule_LoadUnsigned(rdb);
        uint64_t relation = RedisModule_LoadUnsigned(rdb);
        assert(Graph_ConnectNodes(g, srcId, destId, relation, &e));
        _RdbLoadEntity(rdb, (GraphEntity*)&e);
    }    
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

void _RdbSaveEntity(RedisModuleIO *rdb, const Entity *e) {
    /* Format:
     * #properties N
     * (name, value type, value) X N  */

    RedisModule_SaveUnsigned(rdb, e->prop_count);

    for(int i = 0; i < e->prop_count; i++) {
        EntityProperty prop = e->properties[i];
        RedisModule_SaveStringBuffer(rdb, prop.name, strlen(prop.name) + 1);
        _RdbSaveSIValue(rdb, &prop.value);
    }
}

void _RdbSaveNodes(RedisModuleIO *rdb, const Graph *g) {
    /* Format:
     * #nodes
     *      ID
     *      #labels M
     *      (labels) X M
     *      #properties N
     *      (name, value type, value) X N */

    // #Nodes
    RedisModule_SaveUnsigned(rdb, Graph_NodeCount(g));

    Entity *e;
    DataBlockIterator *iter = Graph_ScanNodes(g);
    while((e = (Entity*)DataBlockIterator_Next(iter))) {
        // ID, currently ignored.
        RedisModule_SaveUnsigned(rdb, e->id);

        // #labels, currently only one label per node.
        RedisModule_SaveUnsigned(rdb, 1);

        // (labels) X M
        int l = Graph_GetNodeLabel(g, e->id);
        RedisModule_SaveUnsigned(rdb, l);
        
        // properties N
        // (name, value type, value) X N
        _RdbSaveEntity(rdb, e);
    }

    DataBlockIterator_Free(iter);
}

void _RdbSaveEdges(RedisModuleIO *rdb, const Graph *g) {
    /* Format:
     * #edges (N)
     * {
     *  edge ID, currently not in use.
     *  source node ID
     *  destination node ID
     *  relation type
     * } X N
     * edge properties X N */

    // Sort deleted indicies.
    QSORT(NodeID, g->nodes->deletedIdx, array_len(g->nodes->deletedIdx), ENTITY_ID_ISLT);    

    // #edges (N)
    RedisModule_SaveUnsigned(rdb, Graph_EdgeCount(g));

    for(int r = 0; r < array_len(g->_relations_map); r++) {
        Edge e;
        NodeID src;
        NodeID dest;
        EdgeID edgeID;
        GrB_Matrix M = g->_relations_map[r];
        TuplesIter *it = TuplesIter_new(M);

        while(TuplesIter_next(it, &dest, &src) == TuplesIter_OK) {
            GrB_Matrix_extractElement_UINT64(&edgeID, M, dest, src);
            // Edge ID.
            RedisModule_SaveUnsigned(rdb, edgeID);
            // Source node ID.
            src = _updatedID(g->nodes->deletedIdx, src);
            RedisModule_SaveUnsigned(rdb, src);
            // Destination node ID.
            dest = _updatedID(g->nodes->deletedIdx, dest);
            RedisModule_SaveUnsigned(rdb, dest);
            // Relation type.
            RedisModule_SaveUnsigned(rdb, r);
            // Edge properties.
            Graph_GetEdge(g, edgeID, &e);
            _RdbSaveEntity(rdb, e.entity);
        }

        TuplesIter_free(it);
    }
}

void RdbSaveGraph(RedisModuleIO *rdb, void *value) {
    /* Format:
     * #nodes
     *      ID
     *      #labels M
     *      (labels) X M
     *      #properties N
     *      (name, value type, value) X N
     *
     * #edges
     *      edge ID, currently not in use.
     *      relation type
     *      source node ID
     *      destination node ID
     *      #properties N
     *      (name, value type, value) X N
     */

    Graph *g = (Graph *)value;

    // Dump nodes.
    _RdbSaveNodes(rdb, g);

    // Dump edges.
    _RdbSaveEdges(rdb, g);
}

void RdbLoadGraph(RedisModuleIO *rdb, Graph *g) {
     /* Format:
     * #nodes
     *      #labels M
     *      (labels) X M
     *      #properties N
     *      (name, value type, value) X N
     *
     * #edges
     *      relation type
     *      source node ID
     *      destination node ID
     *      #properties N
     *      (name, value type, value) X N
     */

    // Load nodes.
    _RdbLoadNodes(rdb, g);

    // Load edges.
    _RdbLoadEdges(rdb, g);

    // Flush all pending changes to graphs.
    GrB_wait();
}
