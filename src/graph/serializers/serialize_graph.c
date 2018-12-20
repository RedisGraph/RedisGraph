/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <assert.h>
#include "../graph.h"
#include "serialize_graph.h"
#include "../../util/arr.h"
#include "../../util/qsort.h"
#include "../../../deps/GraphBLAS/Include/GraphBLAS.h"

// Use a modified binary search to find the number of elements in
// the array less than the input ID.
// This value is the number the input ID should be decremented by.
static uint64_t shiftCount(uint64_t *array, NodeID id) {
    uint32_t deletedIndicesCount = array_len(array);
    uint32_t left = 0;
    uint32_t right = deletedIndicesCount;
    uint32_t pos;
    while(left < right) {
        pos = (right + left) / 2;
        if(array[pos] < id) {
            left = pos + 1;
        } else {
            right = pos;
        }
    }
    return left;
}

static NodeID _updatedID(uint64_t *array, NodeID id) {
    uint32_t itemCount = array_len(array);
    if(itemCount == 0) {
        // No deleted elements; don't modify ID
        return id;
    } else if(id > array[itemCount-1]) {
        // ID is greater than all deleted elements, reduce by deleted count
        return id - itemCount;
    } else if(id < array[0]) {
        // ID is lower than all deleted elements, don't modify
        return id;
    } else {
        // Shift ID left by number of deleted IDs lower than it
        return id - shiftCount(array, id);
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
    } else if (t == T_NULL) {
        return SI_NullVal();
    } else {
        char *strVal = RedisModule_LoadStringBuffer(rdb, NULL);
        // Transfer ownership of the heap-allocated strVal to the
        // newly-created SIValue
        return SI_TransferStringVal(strVal);
    }
}

void _RdbLoadEntity(RedisModuleIO *rdb, GraphEntity *e) {
    /* Format:
     * #properties N
     * (name, value type, value) X N
    */
    uint64_t propCount = RedisModule_LoadUnsigned(rdb);
    if(!propCount) return;

    char *propName[propCount];
    SIValue propValue[propCount];

    for(int i = 0; i < propCount; i++) {
        propName[i] = RedisModule_LoadStringBuffer(rdb, NULL);
        propValue[i] = _RdbLoadSIValue(rdb);
    }
    GraphEntity_Add_Properties(e, propCount, propName, propValue);

    // GraphEntity_Add_Properties, duplicates property name,
    // until a better solution is found for handling entities properties
    // we have no choice but to free property name.
    for(int i = 0; i < propCount; i++) RedisModule_Free(propName[i]);
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

    Graph_AllocateNodes(g, nodeCount);
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

    Graph_AllocateEdges(g, edgeCount);
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
    } else if (v->type == T_NULL) {
        return; // No data beyond the type needs to be encoded for a NULL value.
    } else if (v->type & SI_STRING) {
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

    // Sort deleted indices.
    QSORT(NodeID, g->nodes->deletedIdx, array_len(g->nodes->deletedIdx), ENTITY_ID_ISLT);    

    // #edges (N)
    RedisModule_SaveUnsigned(rdb, Graph_EdgeCount(g));

    for(int r = 0; r < array_len(g->_relations_map); r++) {
        Edge e;
        NodeID src;
        NodeID dest;
        EdgeID edgeID;
        GrB_Matrix M = g->_relations_map[r];
        GxB_MatrixTupleIter *it;
        GxB_MatrixTupleIter_new(&it, M);

        while(true) {
            bool depleted = false;
            GxB_MatrixTupleIter_next(it, &dest, &src, &depleted);
            if(depleted) break;

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

        GxB_MatrixTupleIter_free(it);
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

    // While loading the graph, minimize matrix realloc and synchronization calls.
    Graph_SetMatrixPolicy(g, RESIZE_TO_CAPACITY);

    // Load nodes.
    _RdbLoadNodes(rdb, g);

    // Load edges.
    _RdbLoadEdges(rdb, g);

    // Revert to default synchronization behavior
    Graph_SetMatrixPolicy(g, SYNC_AND_MINIMIZE_SPACE);

    // Resize and flush all pending changes to matrices.
    Graph_ApplyAllPending(g);
}
