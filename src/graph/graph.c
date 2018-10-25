/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <assert.h>

#include "graph.h"
#include "serializers/graph_type.h"
#include "../util/qsort.h"
#include "../GraphBLASExt/tuples_iter.h"
#include "../GraphBLASExt/GxB_Pending.h"
#include "../util/rmalloc.h"


/*========================= Graph utility functions ========================= */

/* Acquire mutex. */
void _Graph_EnterCriticalSection(Graph *g) {
    pthread_mutex_lock(&g->_mutex);
}

/* Release mutex. */
void _Graph_LeaveCriticalSection(Graph *g) {
    pthread_mutex_unlock(&g->_mutex);
}

/* Force execution of all pending operations on a matrix. */
static inline void _Graph_ApplyPending(GrB_Matrix m) {
    GrB_Index nvals;
    GrB_Matrix_nvals(&nvals, m);
}

/* module.c global indicating whether the process is locked by a writer. */
extern bool _writelocked;

/* Resize given matrix, such that its number of row and columns
 * matches the number of nodes in the graph. Also, synchronize
 * matrix to execute any pending operations. */
void _Graph_SynchronizeMatrix(const Graph *g, GrB_Matrix m) {
    GrB_Index n_rows;
    GrB_Matrix_nrows(&n_rows, m);

    // If the graph belongs to one thread, we don't need to flush pending operations
    // or lock the mutex.
    if (_writelocked) {
        if (n_rows != Graph_NodeCount(g)) {
            assert(GxB_Matrix_resize(m, Graph_NodeCount(g), Graph_NodeCount(g)) == GrB_SUCCESS);
        }
        return;
    }
    // If the matrix has pending operations or requires
    // a resize, enter critical section.
    if(GxB_Matrix_Pending(m) || (n_rows != Graph_NodeCount(g))) {
        _Graph_EnterCriticalSection((Graph *)g);
        // Double-check if resize is necessary.
        GrB_Matrix_nrows(&n_rows, m);
        if(n_rows != Graph_NodeCount(g))
          assert(GxB_Matrix_resize(m, Graph_NodeCount(g), Graph_NodeCount(g)) == GrB_SUCCESS);

        // Flush changes to matrices if necessary.
        if (GxB_Matrix_Pending(m)) _Graph_ApplyPending(m);

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
        M = Graph_GetRelation(g, i);

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

/* Whenever an edge is replaced by another edge 
 * update the replacement edge ID. */
void _Graph_ReplaceDeletedEdge(Graph *g, EdgeID replacement, EdgeID to_delete) {
    Edge *e = Graph_GetEdge(g, replacement);
    e->id = to_delete;
}

void _Graph_ReplaceDeletedNode(Graph *g, GrB_Vector zero, NodeID replacement, NodeID to_delete) {
    Node *deletedNode = Graph_GetNode(g, to_delete);
    Node *replacementNode = Graph_GetNode(g, replacement);
    
    // Get edges of replacement node.
    Vector *edges = NewVector(Edge*, 32);
    Graph_GetNodeEdges(g, replacementNode, edges, GRAPH_EDGE_DIR_BOTH, GRAPH_NO_RELATION);
    size_t edgeCount = Vector_Size(edges);

    // Update replacement node ID.
    replacementNode->id = to_delete;

    /* Update replacement edges;
     * forevery incoming/outgoing edge of replacement node
     * remove edge from its current position within graph
     * hashtable, once removed, we're allowed to update edge
     * composite key and re-add it to graph hashtable. */    
    for(int i = 0; i < edgeCount; i++) {
        Edge *e;
        Vector_Get(edges, i, &e);

        HASH_DEL(g->_edgesHashTbl, e);

        /* Update edge source/dest node,
         * as we've changed replacement node id. */
        if(Edge_GetSrcNodeID(e) == replacement) {
            Edge_SetSrcNode(e, replacementNode);
        } else {
            Edge_SetDestNode(e, replacementNode);
        }

        HASH_ADD(hh, g->_edgesHashTbl, edgeDesc, sizeof(e->edgeDesc), e);
    }

    // Update label matrices.
    for (int i = 0; i < g->label_count; i ++) {
        bool src_has_label = false;
        bool dest_has_label = false;
        GrB_Matrix M = Graph_GetLabel(g, i);
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

/* Initialize edge and store it within graph's edge hashtable
 * edge key within the hashtable is composed of:
 * 1. Edge relation ID
 * 2. Edge source node ID
 * 3. Edge destination node ID */
int _Graph_InitEdge(Graph *g, Edge *e, EdgeID id, Node *src, Node *dest, int r) {
    // Insert only if edge not already in hashtable.
    Edge *edge;
    EdgeDesc lookupKey = {src->id, dest->id, r};
    HASH_FIND(hh, g->_edgesHashTbl, &lookupKey, sizeof(EdgeDesc), edge);

    if(edge) {
        /* TODO: Currently we only support a single edge of type T
         * between tow nodes. */
        return 0;
    }

    // Set edge composite ID.
    e->id = id;
    Edge_SetSrcNode(e, src);
    Edge_SetDestNode(e, dest);
    Edge_SetRelationID(e, r);

    // Store edge within graph's edge hashtable
    HASH_ADD(hh, g->_edgesHashTbl, edgeDesc, sizeof(EdgeDesc), e);
    return 1;
}

/* Removes an edge from Graph's hashtable and updates 
 * graph relevent matrices. */
void _Graph_DeleteEdge(Graph *g, Edge *e) {
    // Remove edge from hashtable.
    HASH_DEL(g->_edgesHashTbl, e);

    // Update relation matrices.
    int r = Edge_GetRelationID(e);
    NodeID src_id = Edge_GetSrcNodeID(e);
    NodeID dest_id = Edge_GetDestNodeID(e);

    GrB_Matrix M = Graph_GetRelation(g, r);
    _Graph_ClearMatrixEntry(g, M, src_id, dest_id);

    // See if source is connected to destination with additional edges.
    bool connected = false;
    for(int i = 0; i < g->relation_count; i++) {
        M = Graph_GetRelation(g, i);
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

/*================================ Graph API ================================ */
Graph *Graph_New(size_t n) {
    assert(n > 0);
    Graph *g = rm_malloc(sizeof(Graph));

    // TODO Our node iterators always cast the elements of this
    // to Nodes, but only the GraphEntity portion can be safely accessed
    g->nodes = DataBlock_New(n, sizeof(GraphEntity));
    g->edges = DataBlock_New(n, sizeof(Edge));
    g->_edgesHashTbl = NULL;            // Init to NULL, required by uthash.
    g->relation_count = 0;
    g->label_count = 0;
    g->_relations = rm_malloc(sizeof(GrB_Matrix) * GRAPH_DEFAULT_RELATION_CAP);
    g->_labels = rm_malloc(sizeof(GrB_Matrix) * GRAPH_DEFAULT_LABEL_CAP);
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
    _Graph_SynchronizeMatrix(g, g->adjacency_matrix);

    if(labels) {
        for(size_t idx = 0; idx < n; idx++) {
            int l = labels[idx];
            if(l != GRAPH_NO_LABEL) {
                GrB_Matrix m = Graph_GetLabel(g, l);
                GrB_Matrix_setElement_BOOL(m, true, node_id, node_id);
            }
            node_id++;
        }
    }
}

void Graph_ConnectNodes(Graph *g, EdgeDesc *connections, size_t connectionCount, DataBlockIterator **edges) {
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
        int r = conn.relationId;        
        if(_Graph_InitEdge(g, e, edgeID++, srcNode, destNode, r)) {
            // Columns represent source nodes, rows represent destination nodes.
            GrB_Matrix M = Graph_GetRelation(g, r);
            GrB_Matrix_setElement_BOOL(adj, true, conn.destId, conn.srcId);
            GrB_Matrix_setElement_BOOL(M, true, conn.destId, conn.srcId);
        }
    }

    /* If access to newly created edges is requested
     * reset iterator pass it back to caller. */
    if(edges) {
        *edges = iter;
        DataBlockIterator_Reset(*edges);
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

void Graph_GetEdgesConnectingNodes(const Graph *g, NodeID src, NodeID dest, int relation, Vector *edges) {
    assert(g && src < Graph_NodeCount(g) && dest < Graph_NodeCount(g) && edges);
    Edge *e;
    EdgeDesc lookupKey = {src, dest, relation};
    
    // Search for edges.
    if(relation != GRAPH_NO_RELATION) {
        // Relation type specified.
        HASH_FIND(hh, g->_edgesHashTbl, &lookupKey, sizeof(EdgeDesc), e);
        if(e) Vector_Push(edges, e);
    } else {
        // Relation type missing, scan through each edge type.
        for(int r = 0; r < g->relation_count; r++) {
            // Update lookup relation id.
            lookupKey.relationId = r;
            // See if there's an edge of type 'r' connecting source to destination.
            HASH_FIND(hh, g->_edgesHashTbl, &lookupKey, sizeof(EdgeDesc), e);
            if(e) Vector_Push(edges, e);
        }
    }
}

/* Retrieves all either incoming or outgoing edges 
 * to/from given node N, depending on given direction. */
void Graph_GetNodeEdges(const Graph *g, const Node *n, Vector *edges, GRAPH_EDGE_DIR dir, int edgeType) {
    assert(g && n && edges);
    NodeID srcNodeID;
    NodeID destNodeID;
    TuplesIter *tupleIter = NULL;
    size_t nodeCount = Graph_NodeCount(g);
    GrB_Matrix M = Graph_GetAdjacencyMatrix(g);

    // Outgoing.
    if(dir == GRAPH_EDGE_DIR_OUTGOING || dir == GRAPH_EDGE_DIR_BOTH) {
        srcNodeID = n->id;
        GrB_Vector outgoing;
        GrB_Vector_new(&outgoing, GrB_BOOL, nodeCount);
        GrB_Col_extract(outgoing, NULL, NULL, M, GrB_ALL, nodeCount, n->id, NULL);

        tupleIter = TuplesIter_new((GrB_Matrix)outgoing);
        while(TuplesIter_next(tupleIter, &destNodeID, NULL) != TuplesIter_DEPLETED)
            Graph_GetEdgesConnectingNodes(g, n->id, destNodeID, edgeType, edges);

        GrB_Vector_free(&outgoing);
        TuplesIter_free(tupleIter);
    }

    // Incoming.
    if(dir == GRAPH_EDGE_DIR_INCOMING || dir == GRAPH_EDGE_DIR_BOTH) {
        GrB_Vector incoming;
        GrB_Vector_new(&incoming, GrB_BOOL, nodeCount);

        GrB_Descriptor desc;
        GrB_Descriptor_new(&desc);
        GrB_Descriptor_set(desc, GrB_INP0, GrB_TRAN);        
        
        GrB_Col_extract(incoming, NULL, NULL, M, GrB_ALL, nodeCount, n->id, desc);
        GrB_Descriptor_free(&desc);

        tupleIter = TuplesIter_new((GrB_Matrix)incoming);
        while(TuplesIter_next(tupleIter, &srcNodeID, NULL) != TuplesIter_DEPLETED)
            Graph_GetEdgesConnectingNodes(g, srcNodeID, n->id, edgeType, edges);

        GrB_Vector_free(&incoming);
        TuplesIter_free(tupleIter);
    }
}

void _Graph_DeleteEntities(Graph *g, EntityID *IDs, size_t IDCount, DataBlock *entityStore) {
    int post_delete_count = entityStore->itemCount - IDCount;

    // Track the highest remaining ID in the graph
    EntityID id_to_save = entityStore->itemCount - 1;

    // Track the highest ID scheduled for deletion that is less than id_to_save
    size_t largest_delete_idx = IDCount - 1;
    EntityID largest_delete = IDs[largest_delete_idx];

    GrB_Vector zero;
    GrB_Vector_new(&zero, GrB_BOOL, Graph_NodeCount(g));

    // Track the lowest ID scheduled for deletion as the destination slot for
    // id_to_save
    int id_to_replace_idx = 0;
    EntityID id_to_replace;

    /* The outer while loop iterates over IDs to be deleted, starting with the
     * lowest, until reaching an ID greater than or equal to the post-delete entity count.
     * Once this condition is met, we've guaranteed that all IDs below the count are
     * not marked for deletion, and all entities above or equal are, and can thus be removed
     * with matrix resizes. */
    while ((id_to_replace = IDs[id_to_replace_idx]) < post_delete_count) {
        /* Track the largest deletion candidate lower than the next ID we plan to save.
         * (This loop will frequently not trigger at all.) */
        while (id_to_save < largest_delete) {
            largest_delete = IDs[--largest_delete_idx];
        }
        // Ensure that we don't save any entity scheduled for deletion
        if (id_to_save == largest_delete) {
            id_to_save --;
            continue;
        }

        // Perform all necessary entity substitution.
        if(entityStore == g->nodes) {
            // Substitute entity within storage and adjacency and label matrices.
            _Graph_ReplaceDeletedNode(g, zero, id_to_save, id_to_replace);
        } else {
            // Update edge id.
            Edge *e = Graph_GetEdge(g, id_to_save);
            e->id = id_to_replace;
            DataBlock_CopyItem(entityStore, id_to_save, id_to_replace);
        }

        // A swap has been made, so we'll update our source and destination indices.
        id_to_replace_idx ++;
        if (id_to_replace_idx >= IDCount) break;
        id_to_save --;
    }

    // Free all deleted entities from datablock
    DataBlock_FreeTop(entityStore, IDCount);

    GrB_Vector_free(&zero);
}

/* Accepts a *sorted* array of IDs for edges to be deleted. */
void Graph_DeleteEdges(Graph *g, EdgeID *IDs, size_t IDCount) {
    for(int i = 0; i < IDCount; i++) {
        Edge *e = Graph_GetEdge(g, IDs[i]);
        // Removes edge from graph hashtable and updates relevent matrices.
        _Graph_DeleteEdge(g, e);
    }

    // Remove edges from datablock.
    _Graph_DeleteEntities(g, IDs, IDCount, g->edges);
}

/* Accepts a *sorted* array of IDs for nodes to be deleted.
 * The deletion is performed by swapping higher-ID nodes not scheduled
 * for deletion into lower vacant positions, until all IDs greater than
 * the updated node count are scheduled for deletion. The adjacency matrix
 * is then resized to remove these. */
void Graph_DeleteNodes(Graph *g, NodeID *IDs, size_t IDCount) {
    assert(g && IDs);
    if(IDCount == 0) return;

    /* Delete incoming/outgoing edges of deleted nodes;
     * For every node marked for deletion, get a list of 
     * both incoming and outgoing edges and delete thoese edges. */
    Vector *edges = NewVector(Edge*, 32);
    for(int i = 0; i < IDCount; i++) {
        Node *n = Graph_GetNode(g, IDs[i]);
        Graph_GetNodeEdges(g, n, edges, GRAPH_EDGE_DIR_BOTH, GRAPH_NO_RELATION);
    }

    // Compose a sorted array of edge ids.
    Edge *e;
    size_t edgeCount = Vector_Size(edges);
    EdgeID *edgeIDs = malloc(sizeof(EdgeID) * edgeCount);
    EdgeID *dupFreeIDs = malloc(sizeof(EdgeID) * edgeCount);
    for(size_t i = 0; i < edgeCount; i++) {
        Vector_Get(edges, i, &e);
        edgeIDs[i] = e->id;
    }

    // Sort and remove duplicates.
    QSORT(EdgeID, edgeIDs, edgeCount, ENTITY_ID_ISLT);

    // Remove duplicates.
    size_t j = 0;  // Index into dupFreeIDs.
    for(int i = 0; i < edgeCount; i++) {
        EdgeID current = edgeIDs[i];
        // Skip duplicates.
        while(i < (edgeCount-1) && current == edgeIDs[i+1]) i++;
        dupFreeIDs[j++] = current;
    }

    Graph_DeleteEdges(g, dupFreeIDs, j);
    _Graph_DeleteEntities(g, IDs, IDCount, g->nodes);

    // Force matrix resizing.
    _Graph_SynchronizeMatrix(g, g->adjacency_matrix);

    // Cleanup.
    free(edgeIDs);
    free(dupFreeIDs);
    Vector_Free(edges);
}

void Graph_LabelNodes(Graph *g, NodeID start_node_id, NodeID end_node_id, int label) {
    assert(g &&
           start_node_id < Graph_NodeCount(g) &&
           start_node_id >= 0 &&
           start_node_id <= end_node_id &&
           end_node_id < Graph_NodeCount(g));

    GrB_Matrix m = Graph_GetLabel(g, label);
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

int Graph_AddLabel(Graph *g) {
    assert(g);
    // The array capacity is guaranteed by the GraphContext calling function.
    GrB_Matrix_new(&g->_labels[g->label_count++], GrB_BOOL, _Graph_NodeCap(g), _Graph_NodeCap(g));
    return g->label_count-1;
}

int Graph_AddRelationType(Graph *g) {
    assert(g);
    // The array capacity is guaranteed by the GraphContext calling function.
    GrB_Matrix_new(&g->_relations[g->relation_count++], GrB_BOOL, _Graph_NodeCap(g), _Graph_NodeCap(g));
    return g->relation_count-1;
}

GrB_Matrix Graph_GetAdjacencyMatrix(const Graph *g) {
    assert(g);
    GrB_Matrix m = g->adjacency_matrix;
    _Graph_SynchronizeMatrix(g, m);
    return m;
}

GrB_Matrix Graph_GetLabel(const Graph *g, int label_idx) {
    assert(g && label_idx < g->label_count);
    GrB_Matrix m = g->_labels[label_idx];
    _Graph_SynchronizeMatrix(g, m);
    return m;
}

GrB_Matrix Graph_GetRelation(const Graph *g, int relation_idx) {
    assert(g && (relation_idx == GRAPH_NO_RELATION || relation_idx < g->relation_count));
    GrB_Matrix m;

    if(relation_idx == GRAPH_NO_RELATION) {
        m = Graph_GetAdjacencyMatrix(g);
    } else {
        m = g->_relations[relation_idx];
        _Graph_SynchronizeMatrix(g, m);
    }
    return m;
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
        M = Graph_GetRelation(g, i);
        GrB_Matrix_nvals(&nvals, M);
    }

    for(int i = 0; i < g->label_count; i++) {
        M = Graph_GetLabel(g, i);
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
        m = Graph_GetRelation(g, i);
        GrB_Matrix_free(&m);
    }
    rm_free(g->_relations);

    // Free matrices.
    for(int i = 0; i < g->label_count; i++) {
        m = Graph_GetLabel(g, i);
        GrB_Matrix_free(&m);
    }
    rm_free(g->_labels);

    DataBlockIterator *it = Graph_ScanNodes(g);
    GraphEntity *node;
    while ((node = (GraphEntity*)DataBlockIterator_Next(it)) != NULL) {
      FreeGraphEntity(node);
    }
    // Free node iterator.
    DataBlockIterator_Free(it);

    it = Graph_ScanEdges(g);
    Edge *edge;
    while ((edge = (Edge*)DataBlockIterator_Next(it)) != NULL) {
      Edge_Free(edge);
    }

    // Free edge iterator.
    DataBlockIterator_Free(it);
    // Free node blocks.
    DataBlock_Free(g->nodes);
    // Free the edges hash table before modifying the edge blocks.
    HASH_CLEAR(hh,g->_edgesHashTbl);
    // Free the edge blocks.
    DataBlock_Free(g->edges);
    pthread_mutex_destroy(&g->_mutex);
    rm_free(g);
}
