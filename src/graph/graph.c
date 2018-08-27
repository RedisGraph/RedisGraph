/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "graph.h"
#include "graph_type.h"
#include "assert.h"
#include "../arithmetic/tuples_iter.h"


/*========================= Graph utility functions ========================= */

/* Acquire mutex. */
void _Graph_EnterCriticalSection(Graph *g) {
    pthread_mutex_lock(&g->_mutex);
}

/* Release mutex. */
void _Graph_LeaveCriticalSection(Graph *g) {
    pthread_mutex_unlock(&g->_mutex);
}

/* Resize given matrix, such that its number of row and columns
 * matches the number of nodes in the graph. */
void _Graph_ResizeMatrix(const Graph *g, GrB_Matrix m) {
    GrB_Index n_rows;

    GrB_Matrix_nrows(&n_rows, m);
    if(n_rows != Graph_NodeCount(g)) {
        _Graph_EnterCriticalSection((Graph *)g);
        {
            // Double check now that we're in critical section.
            GrB_Matrix_nrows(&n_rows, m);
            if(n_rows != Graph_NodeCount(g))
                assert(GxB_Matrix_resize(m, Graph_NodeCount(g), Graph_NodeCount(g)) == GrB_SUCCESS);
        }
        _Graph_LeaveCriticalSection((Graph *)g);
    }
}

// Return number of nodes graph can contain.
size_t _Graph_NodeCap(const Graph *g) {
    return g->nodes->itemCap;
}

// Resize graph's node array to contain at least n nodes.
void _Graph_AddNodes(Graph *g, size_t n, DataBlockIterator **it) {
    DataBlock_AddItems(g->nodes, n, it);
}

// Resize graph's edge array to contain at least n edges.
void _Graph_AddEdges(Graph *g, size_t n, DataBlockIterator **it) {
    DataBlock_AddItems(g->edges, n, it);
}

/* Relocate src row and column, overriding dest. */
void _Graph_MigrateRowCol(Graph *g, int src, int dest) {
    GrB_Vector row;
    GrB_Vector col;
    GrB_Vector zero;
    GrB_Descriptor desc;
    GrB_Index nrows = Graph_NodeCount(g);
    GrB_Matrix M = Graph_GetAdjacencyMatrix(g);

    GrB_Descriptor_new(&desc);
    GrB_Descriptor_set(desc, GrB_INP0, GrB_TRAN);
    // GrB_Descriptor_set(desc, GrB_OUTP, GrB_REPLACE);

    GrB_Vector_new(&row, GrB_BOOL, nrows);
    GrB_Vector_new(&col, GrB_BOOL, nrows);
    GrB_Vector_new(&zero, GrB_BOOL, nrows);

    // Clear dest column.
    GrB_Col_assign(M, NULL, NULL, zero, GrB_ALL, nrows, dest, NULL);

    // Migrate row.
    GrB_Col_extract(row, NULL, NULL, M, GrB_ALL, nrows, src, desc);
    GrB_Row_assign(M, NULL, NULL, row, dest, GrB_ALL, nrows, NULL);

    // Migrate column.
    GrB_Col_extract(col, NULL, NULL, M, GrB_ALL, nrows, src, NULL);
    GrB_Col_assign(M, NULL, NULL, col, GrB_ALL, nrows, dest, NULL);

    for(int i = 0; i < g->relation_count; i++) {
        M = Graph_GetRelationMatrix(g, i);

        // Clear dest column.
        GrB_Col_assign(M, NULL, NULL, zero, GrB_ALL, nrows, dest, NULL);

        // Migrate row.
        GrB_Col_extract(row, NULL, NULL, M, GrB_ALL, nrows, src, desc);
        GrB_Row_assign(M, NULL, NULL, row, dest, GrB_ALL, nrows, NULL);

        // Migrate column.
        GrB_Col_extract(col, NULL, NULL, M, GrB_ALL, nrows, src, NULL);
        GrB_Col_assign(M, NULL, NULL, col, GrB_ALL, nrows, dest, NULL);
    }

    // Clean up
    GrB_Vector_free(&row);
    GrB_Vector_free(&col);
    GrB_Vector_free(&zero);
    GrB_Descriptor_free(&desc);
}

/* Removes a single entry from given matrix. */
void _Graph_ClearMatrixEntry(Graph *g, GrB_Matrix M, GrB_Index src, GrB_Index dest) {
    GrB_Vector mask;
    GrB_Index nrows = Graph_NodeCount(g);
    GrB_Vector_new(&mask, GrB_BOOL, nrows);
    GrB_Vector_setElement_BOOL(mask, true, dest);

    GrB_Vector col;
    GrB_Vector_new(&col, GrB_BOOL, nrows);

    GrB_Descriptor desc;
    GrB_Descriptor_new(&desc);
    GrB_Descriptor_set(desc, GrB_OUTP, GrB_REPLACE);
    GrB_Descriptor_set(desc, GrB_MASK, GrB_SCMP);

    // Extract column src_id.
    GrB_Col_extract(col, mask, NULL, M, GrB_ALL, nrows, src, desc);
    GrB_Col_assign(M, NULL, NULL, col, GrB_ALL, nrows, src, NULL);

    GrB_Descriptor_free(&desc);
    GrB_Vector_free(&col);
    GrB_Vector_free(&mask);
}

/* Deletes all edges connecting source to destination. */
void _Graph_DeleteEdges(Graph *g, NodeID src_id, NodeID dest_id) {
    GrB_Matrix M = Graph_GetAdjacencyMatrix(g);
    _Graph_ClearMatrixEntry(g, M, src_id, dest_id);

    // Update relation matrices.
    for(int i = 0; i < g->relation_count; i++) {
        M = Graph_GetRelationMatrix(g, i);
        bool connected = false;
        GrB_Matrix_extractElement_BOOL(&connected, M, dest_id, src_id);
        if(connected)
            _Graph_ClearMatrixEntry(g, M, src_id, dest_id);
    }
}

/* Deletes typed edge connecting source to destination. */
void _Graph_DeleteTypedEdges(Graph *g, NodeID src_id, NodeID dest_id, int relation) {
    bool connected = false;
    GrB_Matrix M = Graph_GetRelationMatrix(g, relation);
    assert(M);

    GrB_Matrix_extractElement_BOOL(&connected, M, dest_id, src_id);
    if(!connected) return;

    _Graph_ClearMatrixEntry(g, M, src_id, dest_id);

    // See if source is connected to destination with additional edges.
    for(int i = 0; i < g->relation_count; i++) {
        M = Graph_GetRelationMatrix(g, i);
        connected = false;
        GrB_Matrix_extractElement_BOOL(&connected, M, dest_id, src_id);
        if(connected) break;
    }

    /* There are no additional edges connecting source to destination
     * Remove edge from THE adjacency matrix. */
    if(!connected) {
        M = Graph_GetAdjacencyMatrix(g);
        _Graph_ClearMatrixEntry(g, M, src_id, dest_id);
    }
}

/* Initialize edge and store it within graph's edge hashtable
 * edge key within the hashtable is composed of:
 * 1. Edge relation ID
 * 2. Edge source node ID
 * 3. Edge destination node ID */
void _Graph_InitEdge(Graph *g, Edge *e, EdgeID id, NodeID srcId, NodeID destId, int r) {

    // Set edge composite ID.
    e->id = id;
    e->src_id = srcId;
    e->dest_id = destId;
    e->relationship_id = r;

    // Insert only if edge not already in hashtable.
    Edge *edge;
    size_t edgeCount = 1;
    Graph_GetEdgesConnectingNodes(g, srcId, destId, r, &edge, &edgeCount);
    if(edgeCount == 0) {
        /* Store edge within graph's edge hashtable,
         * calculate the key length including padding using formula:
         * offset of last key field + size of last key field - offset of first key field */
        unsigned keylen = offsetof(Edge, relationship_id) + sizeof(e->relationship_id) - offsetof(Edge, src_id);
        HASH_ADD(hh, g->_edgesHashTbl, src_id, keylen, e);
    }
}

/*================================ Graph API ================================ */
Graph *Graph_New(size_t n) {
    assert(n > 0);
    Graph *g = malloc(sizeof(Graph));

    g->nodes = DataBlock_New(n, sizeof(Node));
    g->edges = DataBlock_New(n, sizeof(Edge));
    g->_edgesHashTbl = NULL;            // Init to NULL, required by uthash.
    g->relation_cap = GRAPH_DEFAULT_RELATION_CAP;
    g->relation_count = 0;
    g->label_cap = GRAPH_DEFAULT_LABEL_CAP;
    g->label_count = 0;
    g->_relations = malloc(sizeof(GrB_Matrix) * g->relation_cap);
    g->_labels = malloc(sizeof(GrB_Matrix) * g->label_cap);
    GrB_Matrix_new(&g->adjacency_matrix, GrB_BOOL, _Graph_NodeCap(g), _Graph_NodeCap(g));

    /* TODO: We might want a mutex per matrix,
     * such that when a thread is resizing matrix A
     * another thread could be resizing matrix B. */
    assert(pthread_mutex_init(&g->_mutex, NULL) == 0);

    return g;
}

Graph *Graph_Get(RedisModuleCtx *ctx, RedisModuleString *graph_name) {
    Graph *g = NULL;

    RedisModuleKey *key = RedisModule_OpenKey(ctx, graph_name, REDISMODULE_WRITE);
    if (RedisModule_ModuleTypeGetType(key) == GraphRedisModuleType) {
        g = RedisModule_ModuleTypeGetValue(key);
    }

    RedisModule_CloseKey(key);
    return g;
}

size_t Graph_NodeCount(const Graph *g) {
    assert(g);
    return g->nodes->itemCount;
}

size_t Graph_EdgeCount(const Graph *g) {
    assert(g);
    return g->edges->itemCount;
}

void Graph_CreateNodes(Graph* g, size_t n, int* labels, DataBlockIterator **it) {
    assert(g);

    NodeID node_id = (NodeID)Graph_NodeCount(g);

    _Graph_AddNodes(g, n, it);
    _Graph_ResizeMatrix(g, g->adjacency_matrix);

    if(labels) {
        for(size_t idx = 0; idx < n; idx++) {
            int l = labels[idx];
            if(l != GRAPH_NO_LABEL) {
                GrB_Matrix m = Graph_GetLabelMatrix(g, l);
                GrB_Matrix_setElement_BOOL(m, true, node_id, node_id);
            }
            node_id++;
        }
    }
}

void Graph_ConnectNodes(Graph *g, ConnectionDesc *connections, size_t connectionCount, DataBlockIterator **it) {
    assert(g && connections);

    DataBlockIterator *iter;
    GrB_Matrix adj = Graph_GetAdjacencyMatrix(g);

    EdgeID edgeID = Graph_EdgeCount(g);
    _Graph_AddEdges(g, connectionCount, &iter);

    // Update graph's adjacency matrices, setting mat[dest,src] to 1.
    for(size_t i = 0; i < connectionCount; i++) {
        // Create, initialize and store edge.
        Edge *e = (Edge*)DataBlockIterator_Next(iter);
        ConnectionDesc conn = connections[i];
        NodeID srcId = conn.srcId;
        NodeID destId = conn.destId;
        int r = conn.relationId;
        _Graph_InitEdge(g, e, edgeID++, srcId, destId, r);

        // Columns represent source nodes, rows represent destination nodes.
        GrB_Matrix_setElement_BOOL(adj, true, destId, srcId);

        if(r != GRAPH_NO_RELATION) {
            // Typed edge.
            GrB_Matrix M = Graph_GetRelationMatrix(g, r);
            GrB_Matrix_setElement_BOOL(M, true, destId, srcId);
        }
    }

    /* If access to newly created edges is requested
     * reset iterator pass it back to caller. */
    if(it) {
        *it = iter;
        DataBlockIterator_Reset(*it);
    } else {
        DataBlockIterator_Free(iter);
    }
}

Node* Graph_GetNode(const Graph *g, NodeID id) {
    assert(g && id < Graph_NodeCount(g));
    Node *n = (Node*)DataBlock_GetItem(g->nodes, id);
    n->id = id;
    return n;
}

Edge *Graph_GetEdge(const Graph *g, EdgeID id) {
    assert(g && id < Graph_EdgeCount(g));
    Edge *e = (Edge*)DataBlock_GetItem(g->edges, id);
    e->id = id;
    return e;
}

void Graph_GetEdgesConnectingNodes(const Graph *g, NodeID src, NodeID dest, int relation, Edge **edges, size_t *edgeCount) {
    assert(g && src < Graph_NodeCount(g) && dest < Graph_NodeCount(g) && edges && edgeCount && *edgeCount > 0);

    Edge *e = NULL;
    size_t edgesFound = 0;  // Number of edges connecting src to dest we've found.

    // Use a fake edge object as a lookup key.
    Edge lookupKey;
    lookupKey.relationship_id = relation;
    lookupKey.src_id = src;
    lookupKey.dest_id = dest;

    // Composite key length.
    unsigned keylen = offsetof(Edge, relationship_id)
                      + sizeof(lookupKey.relationship_id)
                      - offsetof(Edge, src_id);

    // Search for edges.
    if(lookupKey.relationship_id != GRAPH_NO_RELATION) {
        // Relation type specified.
        HASH_FIND(hh, g->_edgesHashTbl, &lookupKey.src_id, keylen, edges[edgesFound]);
        if(edges[edgesFound]) edgesFound += 1;
    } else {
        // Relation type missing, scan through each edge type.
        for(int r = 0; r < g->relation_count && edgesFound < *edgeCount; r++) {
            // Update lookup key relation id.
            lookupKey.relationship_id = r;

            // See if there's an edge of type 'r' connecting source to destination.
            HASH_FIND(hh, g->_edgesHashTbl, &lookupKey.src_id, keylen, edges[edgesFound]);
            if(edges[edgesFound]) edgesFound += 1;
        }
    }

    // Let caller know how many edges we've found.
    *edgeCount = edgesFound;
}

void Graph_DeleteNodes(Graph *g, NodeID *IDs, size_t IDCount) {
    assert(g && IDs);
    if(IDCount == 0) return;

    typedef struct {
        NodeID nodeID;          // Node being deleted.
        NodeID replacementID;   // Node taking over.
        bool delete;            // No need to replace, simply delete.
    } Replacement;

    Replacement *replacements = malloc(sizeof(Replacement) * IDCount);

    /* Allocate replacement candidates. */
    for(size_t i = 0; i < IDCount; i++) {
        replacements[i].replacementID = Graph_NodeCount(g) - (IDCount - i);
        replacements[i].delete = false;
    }

    /* Locate which soon to deleted nodes are also replacement candidates. */
    for(size_t i = 0; i < IDCount; i++) {
        NodeID id = IDs[i];
        if(id >= (Graph_NodeCount(g) - IDCount)) {
            size_t j = IDCount - (Graph_NodeCount(g) - id);
            replacements[j].nodeID = id;
            replacements[j].delete = true;
        }
    }

    /* For nodes marked for deletion which do require a replacement
     * find a replacement which is not marked for quick deletion. */
    for(size_t j = 0, i = 0; i < IDCount; i++) {
        NodeID id = IDs[i];
        // Require a replacement?
        if(id < (Graph_NodeCount(g) - IDCount)) {
            // Locate a valid replacement.
            while(replacements[j].delete) j++;
            replacements[j++].nodeID = id;
        }
    }

    /* Replace rows, columns. */
    for(size_t i = 0; i < IDCount; i++) {
        Replacement r = replacements[i];
        if(!r.delete) {
            _Graph_MigrateRowCol(g, r.replacementID, r.nodeID);
        }
    }

    // Zero vector to clear entire row/column.
    GrB_Vector zero;
    GrB_Vector_new(&zero, GrB_BOOL, Graph_NodeCount(g));

    // Update label matrices.
    for(size_t i = 0; i < g->label_count; i++) {
        GrB_Matrix M = Graph_GetLabelMatrix(g, i);
        for(size_t j = 0; j < IDCount; j++) {
            Replacement r = replacements[j];
            bool srcExists = false;
            bool destExists = false;
            GrB_Matrix_extractElement_BOOL(&srcExists, M, r.replacementID, r.replacementID);
            GrB_Matrix_extractElement_BOOL(&destExists, M, r.nodeID, r.nodeID);

            // Clear, dest.
            if(destExists) {
                if(!srcExists || (srcExists && r.delete) ) {
                    GrB_Col_assign(M, NULL, NULL, zero, GrB_ALL, Graph_NodeCount(g), r.nodeID, NULL);
                }
            }

            // Set dest
            if(!destExists) {
                if(srcExists && !r.delete) {
                    GrB_Matrix_setElement_BOOL(M, true, r.nodeID, r.nodeID);
                }
            }
        }
    }

    /* Replace removed nodes within node blocks. */
    size_t deletedNodeCount = 0;
    for(size_t i = 0; i < IDCount; i++) {
        Replacement r = replacements[i];
        if(r.delete) {
            // No need to perform replacement.
            deletedNodeCount++;
        } else {
            // Override nodeID with replacementID.
            DataBlock_MigrateItem(g->nodes, r.replacementID, r.nodeID);
        }
    }
    DataBlock_FreeTop(g->nodes, deletedNodeCount);

    // Clean up.
    GrB_Vector_free(&zero);
    free(replacements);
}

void Graph_DeleteEdge(Graph *g, NodeID src_id, NodeID dest_id, int relation) {
    assert(src_id < Graph_NodeCount(g) && dest_id < Graph_NodeCount(g));

    // See if there's an edge between src and dest.
    bool connected = false;
    GrB_Matrix M = Graph_GetAdjacencyMatrix(g);
    GrB_Matrix_extractElement_BOOL(&connected, M, dest_id, src_id);

    if(!connected) return;

    if(relation == GRAPH_NO_RELATION) {
        // Remove every edge connecting source to destination.
        _Graph_DeleteEdges(g, src_id, dest_id);
    } else {
        // Remove typed edge connecting source to destination.
        _Graph_DeleteTypedEdges(g, src_id, dest_id, relation);
    }
}

void Graph_LabelNodes(Graph *g, NodeID start_node_id, NodeID end_node_id, int label) {
    assert(g &&
           start_node_id < Graph_NodeCount(g) &&
           start_node_id >= 0 &&
           start_node_id <= end_node_id &&
           end_node_id < Graph_NodeCount(g));

    GrB_Matrix m = Graph_GetLabelMatrix(g, label);
    for(NodeID node_id = start_node_id; node_id <= end_node_id; node_id++) {
        GrB_Matrix_setElement_BOOL(m, true, node_id, node_id);
    }
}

DataBlockIterator *Graph_ScanNodes(const Graph *g) {
    assert(g);
    return DataBlock_Scan(g->nodes);
}

DataBlockIterator *Graph_ScanEdges(const Graph *g) {
    assert(g);
    return DataBlock_Scan(g->edges);
}

int Graph_AddLabelMatrix(Graph *g) {
    assert(g);

    // Make sure we've got room for a new label matrix.
    if(g->label_count == g->label_cap) {
        g->label_cap += 4;   // allocate room for 4 new matrices.
        g->_labels = realloc(g->_labels, g->label_cap * sizeof(GrB_Matrix));
    }

    GrB_Matrix_new(&g->_labels[g->label_count++], GrB_BOOL, _Graph_NodeCap(g), _Graph_NodeCap(g));
    return g->label_count-1;
}

GrB_Matrix Graph_GetAdjacencyMatrix(const Graph *g) {
    assert(g);
    GrB_Matrix m = g->adjacency_matrix;
    _Graph_ResizeMatrix(g, m);
    return m;
}

GrB_Matrix Graph_GetLabelMatrix(const Graph *g, int label_idx) {
    assert(g && label_idx < g->label_count);
    GrB_Matrix m = g->_labels[label_idx];
    _Graph_ResizeMatrix(g, m);
    return m;
}

GrB_Matrix Graph_GetRelationMatrix(const Graph *g, int relation_idx) {
    assert(g && relation_idx < g->relation_count);
    GrB_Matrix m = g->_relations[relation_idx];
    _Graph_ResizeMatrix(g, m);
    return m;
}

int Graph_AddRelationMatrix(Graph *g) {
    assert(g);

    // Make sure we've got room for a new relation matrix.
    if(g->relation_count == g->relation_cap) {
        g->relation_cap += 4;   // allocate room for 4 new matrices.
        g->_relations = realloc(g->_relations, g->relation_cap * sizeof(GrB_Matrix));
    }

    GrB_Matrix_new(&g->_relations[g->relation_count++], GrB_BOOL, _Graph_NodeCap(g), _Graph_NodeCap(g));
    return g->relation_count-1;
}

void Graph_CommitPendingOps(Graph *g) {
    /* GraphBLAS might delay execution of operations to a later stage,
     * here we're forcing GraphBLAS to execute all of its pending operations
     * by asking for the number of entries in each matrix. */

    GrB_Matrix M;
    GrB_Index nvals;

    M = Graph_GetAdjacencyMatrix(g);
    GrB_Matrix_nvals(&nvals, M);

    for(int i = 0; i < g->relation_count; i++) {
        M = Graph_GetRelationMatrix(g, i);
        GrB_Matrix_nvals(&nvals, M);
    }

    for(int i = 0; i < g->label_count; i++) {
        M = Graph_GetLabelMatrix(g, i);
        GrB_Matrix_nvals(&nvals, M);
    }
}

void Graph_Free(Graph *g) {
    assert(g);


    /* TODO: Free nodes, currently we can't free nodes
     * as they are embedded within the chain block, as a result
     * we can't call free on a single node.
     * on the other hand when freeing a query graph, we're able
     * to call free on node. this will be resolved once we'll
     * introduce property stores. */

    // Free each node.
    // Node *node;
    // NodeIterator *it = Graph_ScanNodes(g);
    // while((node = NodeIterator_Next(it)) != NULL) {
    //     Node_Free(node);
    // }
    // NodeIterator_Free(it);

    // Free matrices.
    GrB_Matrix m;
    m = Graph_GetAdjacencyMatrix(g);

    GrB_Matrix_free(&m);
    for(int i = 0; i < g->relation_count; i++) {
        m = Graph_GetRelationMatrix(g, i);
        GrB_Matrix_free(&m);
    }
    free(g->_relations);

    // Free matrices.
    for(int i = 0; i < g->label_count; i++) {
        m = Graph_GetLabelMatrix(g, i);
        GrB_Matrix_free(&m);
    }
    free(g->_labels);

    // Free node blocks.
    DataBlock_Free(g->nodes);
    DataBlock_Free(g->edges);

    pthread_mutex_destroy(&g->_mutex);
    free(g);
}
