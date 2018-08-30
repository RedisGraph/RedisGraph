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

void _Graph_ReplaceDeletedNode(Graph *g, GrB_Vector zero, NodeID replacement, NodeID to_delete) {
    // Update label matrices.
    for (int i = 0; i < g->label_count; i ++) {
        bool src_has_label = false;
        bool dest_has_label = false;
        GrB_Matrix M = Graph_GetLabelMatrix(g, i);
        GrB_Matrix_extractElement_BOOL(&src_has_label, M, replacement, replacement);
        GrB_Matrix_extractElement_BOOL(&dest_has_label, M, to_delete, to_delete);

        if (dest_has_label && !src_has_label) {
            // Zero out the destination column if the deleted node possesses the label and the replacement does not
            assert(GrB_Col_assign(M, NULL, NULL, zero, GrB_ALL, Graph_NodeCount(g), to_delete, NULL) == GrB_SUCCESS);
        } else if (!dest_has_label && src_has_label) {
            // Set the destination column if the replacement possesses the label and the destination does not
            GrB_Matrix_setElement_BOOL(M, true, to_delete, to_delete);
        }
    }

    _Graph_MigrateRowCol(g, replacement, to_delete);
    DataBlock_CopyItem(g->nodes, replacement, to_delete);
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
    // Guarantee src connected to dest.
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
int _Graph_InitEdge(Graph *g, Edge *e, EdgeID id, Node *src, Node *dest, int r) {
    // Insert only if edge not already in hashtable.
    Edge *edge;
    Edge lookupKey;
    memset(&lookupKey, 0, sizeof(Edge));
    Edge_SetSrcNode(&lookupKey, src);
    Edge_SetDestNode(&lookupKey, dest);
    Edge_SetRelationID(&lookupKey, r);
    HASH_FIND(hh, g->_edgesHashTbl, &lookupKey.edgeDesc, sizeof(EdgeDesc), edge);

    if(edge) {
        /* TODO: Currently we only support a single edge of type T
         * between tow nodes. */
        return 0;
    }

    // Set edge composite ID.
    Edge_SetSrcNode(e, src);
    Edge_SetDestNode(e, dest);
    Edge_SetRelationID(e, r);

    // Store edge within graph's edge hashtable
    HASH_ADD(hh, g->_edgesHashTbl, edgeDesc, sizeof(EdgeDesc), e);
    return 1;
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

void Graph_ConnectNodes(Graph *g, EdgeDesc *connections, size_t connectionCount, DataBlockIterator **it) {
    assert(g && connections);

    DataBlockIterator *iter;
    GrB_Matrix adj = Graph_GetAdjacencyMatrix(g);

    EdgeID edgeID = Graph_EdgeCount(g);
    _Graph_AddEdges(g, connectionCount, &iter);

    // Update graph's adjacency matrices, setting mat[dest,src] to 1.
    for(size_t i = 0; i < connectionCount; i++) {
        // Create, initialize and store edge.
        Edge *e = (Edge*)DataBlockIterator_Next(iter);
        EdgeDesc conn = connections[i];
        Node *srcNode = Graph_GetNode(g, conn.srcId);
        Node *destNode = Graph_GetNode(g, conn.destId);
        assert(srcNode && destNode);
        int r = conn.relationId;
        if(_Graph_InitEdge(g, e, edgeID++, srcNode, destNode, r)) {
            // Columns represent source nodes, rows represent destination nodes.
            GrB_Matrix M = Graph_GetRelationMatrix(g, r);
            GrB_Matrix_setElement_BOOL(adj, true, destNode->id, srcNode->id);
            GrB_Matrix_setElement_BOOL(M, true, destNode->id, srcNode->id);
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

void Graph_GetEdgesConnectingNodes(const Graph *g, NodeID src, NodeID dest, int relation, EdgeIterator *it) {
    assert(g && src < Graph_NodeCount(g) && dest < Graph_NodeCount(g) && it);

    size_t edgeCount = 0;
    size_t edgeCap = it->edgeCap;
    Edge **edges = it->edges;
    Node *srcNode = Graph_GetNode(g, src);
    Node *destNode = Graph_GetNode(g, dest);

    // Use a fake edge object as a lookup key.
    Edge lookupKey;
    memset(&lookupKey, 0, sizeof(Edge));
    Edge_SetSrcNode(&lookupKey, srcNode);
    Edge_SetDestNode(&lookupKey, destNode);
    Edge_SetRelationID(&lookupKey, relation);

    // Search for edges.
    if(relation != GRAPH_NO_RELATION) {
        // Relation type specified.
        HASH_FIND(hh, g->_edgesHashTbl, &lookupKey.edgeDesc, sizeof(EdgeDesc), edges[edgeCount]);
        if(edges[edgeCount]) edgeCount += 1;
    } else {
        // Relation type missing, scan through each edge type.
        for(int r = 0; r < g->relation_count; r++) {
            // Update lookup key relation id.
            Edge_SetRelationID(&lookupKey, r);
            // See if there's an edge of type 'r' connecting source to destination.
            HASH_FIND(hh, g->_edgesHashTbl, &lookupKey.edgeDesc, sizeof(EdgeDesc), edges[edgeCount]);
            if(edges[edgeCount]) edgeCount++;
            if(edgeCount >= edgeCap) {
                edgeCap *= 2;
                edges = realloc(edges, sizeof(Edge*) * edgeCap);
            }
        }
    }

    it->edges = edges;
    it->edgeCap = edgeCap;
    it->edgeCount = edgeCount;
}

/* Accepts a *sorted* array of IDs for nodes to be deleted.
 * The deletion is performed by swapping higher-ID nodes not scheduled
 * for deletion into lower vacant positions, until all IDs greater than
 * the updated node count are scheduled for deletion. The adjacency matrix
 * is then resized to remove these. */
void Graph_DeleteNodes(Graph *g, NodeID *IDs, size_t IDCount) {
    assert(g && IDs);
    if(IDCount == 0) return;

    int post_delete_count = Graph_NodeCount(g) - IDCount;

    // Track the highest remaining ID in the graph
    NodeID id_to_save = Graph_NodeCount(g) - 1;

    // Track the highest ID scheduled for deletion that is less than id_to_save
    int largest_delete_idx = IDCount - 1;
    NodeID largest_delete = IDs[largest_delete_idx];

    GrB_Vector zero;
    GrB_Vector_new(&zero, GrB_BOOL, Graph_NodeCount(g));

    // Track the lowest ID scheduled for deletion as the destination slot for
    // id_to_save
    int id_to_replace_idx = 0;
    NodeID id_to_replace;

    /* The outer while loop iterates over IDs to be deleted, starting with the
     * lowest, until reaching an ID greater than or equal to the post-delete node count.
     * Once this condition is met, we've guaranteed that all IDs below the count are
     * not marked for deletion, and all nodes above or equal are, and can thus be removed
     * with matrix resizes. */
    while ((id_to_replace = IDs[id_to_replace_idx]) < post_delete_count) {
        /* Track the largest deletion candidate lower than the next ID we plan to save.
         * (This loop will frequently not trigger at all.) */
        while (id_to_save < largest_delete) {
            largest_delete = IDs[--largest_delete_idx];
        }
        // Ensure that we don't save any nodes scheduled for deletion
        if (id_to_save == largest_delete) {
            id_to_save --;
            continue;
        }

        // Perform all necessary substitutions in node storage and
        // adjacency and label matrices
        _Graph_ReplaceDeletedNode(g, zero, id_to_save, id_to_replace);

        // A swap has been made, so we'll update our source and destination indices.
        id_to_replace_idx ++;
        if (id_to_replace_idx >= IDCount) break;
        id_to_save --;
    }

    // Free all deleted nodes from datablock
    DataBlock_FreeTop(g->nodes, IDCount);
    // Force matrix resizing.
    _Graph_ResizeMatrix(g, g->adjacency_matrix);

    GrB_Vector_free(&zero);
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
    // Free the edges hash table before modifying the edge blocks.
    HASH_CLEAR(hh,g->_edgesHashTbl);
    // Free the edge blocks.
    DataBlock_Free(g->edges);
    pthread_mutex_destroy(&g->_mutex);
    free(g);
}
