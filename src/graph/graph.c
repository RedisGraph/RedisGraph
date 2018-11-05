/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <assert.h>

#include "graph.h"
#include "../util/arr.h"
#include "../util/qsort.h"
#include "../GraphBLASExt/GxB_Delete.h"
#include "../GraphBLASExt/tuples_iter.h"
#include "../GraphBLASExt/GxB_Pending.h"
#include "../util/rmalloc.h"

/*========================= Synchronization functions ========================= */

/* Acquire mutex when a reader thread may modify shared data. */
static inline void _Graph_EnterCriticalSection(Graph *g) {
    pthread_mutex_lock(&g->_mutex);
}

/* Release mutex. */
static inline void _Graph_LeaveCriticalSection(Graph *g) {
    pthread_mutex_unlock(&g->_mutex);
}

/* Acquire a lock that does not restrict access from additional reader threads */
void Graph_AcquireReadLock(Graph *g) {
    pthread_rwlock_rdlock(&g->_rwlock);
}

/* Acquire a lock for exclusive access to this graph's data */
void Graph_AcquireWriteLock(Graph *g) {
    pthread_rwlock_wrlock(&g->_rwlock);
    g->_writelocked = true;
}

/* Release the held lock */
void Graph_ReleaseLock(Graph *g) {
    g->_writelocked = false;
    pthread_rwlock_unlock(&g->_rwlock);
}

/* Force execution of all pending operations on a matrix. */
static inline void _Graph_ApplyPending(GrB_Matrix m) {
    GrB_Index nvals;
    GrB_Matrix_nvals(&nvals, m);
}

/*========================= Graph utility functions ========================= */

/* Allow different operations to choose their matrix synchronization policy.
 * Bulk insertion and whole-graph deletion do not need to resize matrices
 * or force completion of pending GraphBLAS operations. */

/* Do not modify matrices upon retrieval. */
void _DontSynchronizeMatrix(const Graph *g, GrB_Matrix m) {
  return;
}

/* Resize given matrix, such that its number of row and columns
 * matches the number of nodes in the graph. Also, synchronize
 * matrix to execute any pending operations. */
void _SynchronizeMatrix(const Graph *g, GrB_Matrix m) {
    GrB_Index n_rows;
    GrB_Matrix_nrows(&n_rows, m);

    // If the graph belongs to one thread, we don't need to flush pending operations
    // or lock the mutex.
    if (g->_writelocked) {
        if (n_rows != Graph_RequiredMatrixDim(g)) {
            assert(GxB_Matrix_resize(m, Graph_RequiredMatrixDim(g), Graph_RequiredMatrixDim(g)) == GrB_SUCCESS);
        }
        return;
    }

    // If the matrix has pending operations or requires
    // a resize, enter critical section.
    if(GxB_Matrix_Pending(m) || (n_rows != Graph_RequiredMatrixDim(g))) {
        _Graph_EnterCriticalSection((Graph *)g);
        // Double-check if resize is necessary.
        GrB_Matrix_nrows(&n_rows, m);
        if(n_rows != Graph_RequiredMatrixDim(g))
          assert(GxB_Matrix_resize(m, Graph_RequiredMatrixDim(g), Graph_RequiredMatrixDim(g)) == GrB_SUCCESS);

        // Flush changes to matrices if necessary.
        if (GxB_Matrix_Pending(m)) _Graph_ApplyPending(m);

        _Graph_LeaveCriticalSection((Graph *)g);
    }
}

void Graph_SetSynchronization(Graph *g, bool enable) {
    if (enable) {
        g->SynchronizeMatrix = _SynchronizeMatrix;
    } else {
        g->SynchronizeMatrix = _DontSynchronizeMatrix;
    }
}

// Return number of nodes graph can contain.
size_t _Graph_NodeCap(const Graph *g) {
    return g->nodes->itemCap;
}

// Return number of nodes graph can contain.
size_t _Graph_EdgeCap(const Graph *g) {
    return g->edges->itemCap;
}

// Retrieve a relation mapping matrix coresponding to relation_idx
// Make sure matrix is synchronized
// TODO: it might be enough just to resize the relation mapping matrix
// and avoid flushing other pendding changes to it, need to verify
// if resize flush pendding changes.
GrB_Matrix _Graph_GetRelationMap(const Graph *g, int relation_idx) {
    assert(g && relation_idx < array_len(g->_relations_map));
    
    GrB_Matrix m = g->_relations_map[relation_idx];
    g->SynchronizeMatrix(g, m);
    return m; 
}

// Create a new mapping matrix M,
// M[I,J] holds the edge ID connecting node J to I (CSC format)
// assuming _relations_map[K] holds mapping for relation K.
void _Graph_AddRelationMap(Graph *g) {
    GrB_Matrix mapper;
    GrB_Info res = GrB_Matrix_new(&mapper, GrB_UINT64, Graph_RequiredMatrixDim(g), Graph_RequiredMatrixDim(g));
    assert(res == GrB_SUCCESS);
    g->_relations_map = array_append(g->_relations_map, mapper);
}

// Locates edge connecting src to destination.
Entity* _Graph_GetEdgeConnectingNodes(const Graph *g, NodeID src, NodeID dest, int r) {
    assert(g && src < Graph_RequiredMatrixDim(g) && dest < Graph_RequiredMatrixDim(g) && r < Graph_RelationTypeCount(g));

    // relation map, maps (src, dest, r) to edge id.
    GrB_Matrix relationMap = _Graph_GetRelationMap(g, r);

    EdgeID edgeId = 0;
    GrB_Info res = GrB_Matrix_extractElement_UINT64(&edgeId, relationMap, dest, src);
    // No entry at [dest, src], src is not connected to dest with relation R.
    if(res == GrB_NO_VALUE) return NULL;

    Entity *en = DataBlock_GetItem(g->edges, edgeId);
    assert(en);
    return en;
}

static inline Entity *_Graph_GetEntity(const DataBlock *entities, EntityID id) {
    return DataBlock_GetItem(entities, id);
}

/*================================ Graph API ================================ */
Graph *Graph_New(size_t n) {
    assert(n > 0);
    Graph *g = rm_malloc(sizeof(Graph));
    g->nodes = DataBlock_New(n, sizeof(Entity));
    g->edges = DataBlock_New(n, sizeof(Entity));
    g->labels = array_new(GrB_Matrix, GRAPH_DEFAULT_LABEL_CAP);
    g->relations = array_new(GrB_Matrix, GRAPH_DEFAULT_RELATION_CAP);
    g->_relations_map = array_new(GrB_Matrix, GRAPH_DEFAULT_RELATION_CAP);
    GrB_Matrix_new(&g->adjacency_matrix, GrB_BOOL, _Graph_NodeCap(g), _Graph_NodeCap(g));

    // Initialize a read-write lock scoped to the individual graph
    assert(pthread_rwlock_init(&g->_rwlock, NULL) == 0);

    // Force GraphBLAS updates and resize matrices to node count by default
    Graph_SetSynchronization(g, true);

    /* TODO: We might want a mutex per matrix,
     * such that when a thread is resizing matrix A
     * another thread could be resizing matrix B. */
    assert(pthread_mutex_init(&g->_mutex, NULL) == 0);

    return g;
}

// All graph matrices are required to be squared NXN
// where N is Graph_RequiredMatrixDim.
size_t Graph_RequiredMatrixDim(const Graph *g) {
    // Matrix dimensions should be at least:
    // Number of nodes + number of deleted nodes.
    return g->nodes->itemCount + array_len(g->nodes->deletedIdx);
}

size_t Graph_NodeCount(const Graph *g) {
    assert(g);
    return g->nodes->itemCount;
}

size_t Graph_EdgeCount(const Graph *g) {
    assert(g);
    return g->edges->itemCount;
}

int Graph_RelationTypeCount(const Graph *g) {
    return array_len(g->relations);
}

int Graph_LabelTypeCount(const Graph *g) {
    return array_len(g->labels);
}

void Graph_AllocateNodes(Graph* g, size_t n) {
    assert(g);
    DataBlock_Accommodate(g->nodes, n);
}

void Graph_AllocateEdges(Graph *g, size_t n) {
    assert(g);
    DataBlock_Accommodate(g->edges, n);
}

int Graph_GetNode(const Graph *g, NodeID id, Node *n) {
    assert(g);
    n->entity = _Graph_GetEntity(g->nodes, id);
    return (n->entity!=NULL);
}

int Graph_GetEdge(const Graph *g, EdgeID id, Edge *e) {
    assert(g && id < _Graph_EdgeCap(g));
    e->entity = _Graph_GetEntity(g->edges, id);
    return (e->entity!=NULL);
}

int Graph_GetNodeLabel(const Graph *g, NodeID nodeID) {
    assert(g);
    int label = GRAPH_NO_LABEL;
    for(int i = 0; i < array_len(g->labels); i++) {
        bool x = false;
        GrB_Matrix M = Graph_GetLabel(g, i);
        GrB_Info res = GrB_Matrix_extractElement_BOOL(&x, M, nodeID, nodeID);
        if(res == GrB_SUCCESS && x == true) {
            label = i;
            break;
        }
    }

    return label;
}

int Graph_GetEdgeRelation(const Graph *g, Edge *e) {
    assert(g && e);
    NodeID srcNodeID = Edge_GetSrcNodeID(e);
    NodeID destNodeID = Edge_GetDestNodeID(e);

    // Search for relation mapping matrix M, where
    // M[dest,src] == edge ID.
    for(int i = 0; i < array_len(g->_relations_map); i++) {
        EdgeID edgeId = 0;
        GrB_Matrix M = _Graph_GetRelationMap(g, i);
        GrB_Info res = GrB_Matrix_extractElement_UINT64(&edgeId, M, destNodeID, srcNodeID);
        if(res == GrB_SUCCESS && edgeId == ENTITY_GET_ID(e)) {
            Edge_SetRelationID(e, i);
            return i;
        }
    }

    // We must be able to find edge relation.
    assert(false);
    return GRAPH_NO_RELATION;
}

void Graph_GetEdgesConnectingNodes(const Graph *g, NodeID srcID, NodeID destID, int r, Edge **edges) {
    assert(g && r < Graph_RelationTypeCount(g) && edges);

    Edge e;
    Entity *en;
    Node srcNode;
    Node destNode;
    assert(Graph_GetNode(g, srcID, &srcNode));
    assert(Graph_GetNode(g, destID, &destNode));
    e.srcNodeID = srcID;
    e.destNodeID = destID;
    e.src = NULL;
    e.dest = NULL;

    if(r != GRAPH_NO_RELATION) {
        en = _Graph_GetEdgeConnectingNodes(g, srcID, destID, r);
        if(!en) return;
        e.entity = en;
        e.relationId = r;
        *edges = array_append(*edges, e);
    } else {
        // Relation type missing, scan through each edge type.
        int relationCount = Graph_RelationTypeCount(g);
        for(int i = 0; i < relationCount; i++) {
            en = _Graph_GetEdgeConnectingNodes(g, srcID, destID, i);
            if(!en) continue;
            e.entity = en;
            e.relationId = i;
            *edges = array_append(*edges, e);
        }
    }
}

void Graph_CreateNode(Graph* g, int label, Node *n) {
    assert(g);

    NodeID id;
    Entity *en = DataBlock_AllocateItem(g->nodes, &id);
    en->id = id;
    n->entity = en;

    if(label != GRAPH_NO_LABEL) {
        // Try to set matrix at position [id, id]
        // incase of a failure, scale matrix.
        GrB_Matrix m = g->labels[label];
        GrB_Info res = GrB_Matrix_setElement_BOOL(m, true, id, id);
        if(res != GrB_SUCCESS) {
            g->SynchronizeMatrix(g, m);
            assert(GrB_Matrix_setElement_BOOL(m, true, id, id) == GrB_SUCCESS);
        }
    }
}

int Graph_ConnectNodes(Graph *g, NodeID src, NodeID dest, int r, Edge *e) {
    Node srcNode;
    Node destNode;
    GrB_Matrix adj;
    GrB_Matrix relationMat;
    GrB_Matrix relationMapMat;

    assert(Graph_GetNode(g, src, &srcNode));
    assert(Graph_GetNode(g, dest, &destNode));
    assert(g && r < Graph_RelationTypeCount(g));

    // Is src already connected to dest with edge of type r.
    relationMat = Graph_GetRelationMatrix(g, r);
    bool x = false;
    GrB_Info res = GrB_Matrix_extractElement_BOOL(&x, relationMat, dest, src);
    if(res == GrB_SUCCESS && x == true) {
        e->entity = NULL;
        return 0;
    }

    e->srcNodeID = src;
    e->destNodeID = dest;

    EdgeID id;
    Entity *en = DataBlock_AllocateItem(g->edges, &id);
    en->id = id;
    e->entity = en;

    adj = Graph_GetAdjacencyMatrix(g);
    relationMapMat = _Graph_GetRelationMap(g, r);

    // Columns represent source nodes, rows represent destination nodes.
    GrB_Matrix_setElement_BOOL(adj, true, dest, src);
    GrB_Matrix_setElement_BOOL(relationMat, true, dest, src);
    GrB_Matrix_setElement_UINT64(relationMapMat, id, dest, src);
    return 1;
}

/* Retrieves all either incoming or outgoing edges 
 * to/from given node N, depending on given direction. */
void Graph_GetNodeEdges(const Graph *g, const Node *n, GRAPH_EDGE_DIR dir, int edgeType, Edge **edges) {
    assert(g && n && edges);
    NodeID srcNodeID;
    NodeID destNodeID;
    TuplesIter *tupleIter;
    GrB_Matrix M;
    if(edgeType == GRAPH_NO_RELATION) M = Graph_GetAdjacencyMatrix(g);
    else M = Graph_GetRelationMatrix(g, edgeType);

    // Outgoing.
    if(dir == GRAPH_EDGE_DIR_OUTGOING || dir == GRAPH_EDGE_DIR_BOTH) {
        tupleIter = TuplesIter_new(M);
        srcNodeID = ENTITY_GET_ID(n);
        TuplesIter_iterate_column(tupleIter, srcNodeID);
        while(TuplesIter_next(tupleIter, &destNodeID, NULL) != TuplesIter_DEPLETED)
        Graph_GetEdgesConnectingNodes(g, srcNodeID, destNodeID, edgeType, edges);
        TuplesIter_free(tupleIter);
    }

    // Incoming.
    if(dir == GRAPH_EDGE_DIR_INCOMING || dir == GRAPH_EDGE_DIR_BOTH) {
        // TODO: Callers whishing to get Incoming edges to a number of nodes
        // should pass a transposed matrix, as the operations below are costly
        // and we'll perform them forevery node.
        destNodeID = ENTITY_GET_ID(n);
        size_t nRows = Graph_RequiredMatrixDim(g);
        GrB_Vector incoming;
        GrB_Descriptor desc;
        GrB_Vector_new(&incoming, GrB_BOOL, nRows);
        GrB_Descriptor_new(&desc);
        GrB_Descriptor_set(desc, GrB_INP0, GrB_TRAN);
        GrB_Col_extract(incoming, NULL, NULL, M, GrB_ALL, nRows, destNodeID, desc);
        GrB_Descriptor_free(&desc);

        tupleIter = TuplesIter_new((GrB_Matrix)incoming);
        TuplesIter_iterate_column(tupleIter, 0);
        while(TuplesIter_next(tupleIter, &srcNodeID, NULL) != TuplesIter_DEPLETED)
            Graph_GetEdgesConnectingNodes(g, srcNodeID, destNodeID, edgeType, edges);

        GrB_Vector_free(&incoming);
        TuplesIter_free(tupleIter);
    }
}

/* Removes an edge from Graph and updates graph relevent matrices. */
int Graph_DeleteEdge(Graph *g, Edge *e) {
    GrB_Matrix M;
    GrB_Info res;
    int r = Edge_GetRelationID(e);
    NodeID src_id = Edge_GetSrcNodeID(e);
    NodeID dest_id = Edge_GetDestNodeID(e);
    M = Graph_GetRelationMatrix(g, r);

    // Test to see if edge exists.
    bool x = false;
    GrB_Matrix_extractElement_BOOL(&x, M, dest_id, src_id);
    if(!x) return 0;

    res = GxB_Matrix_Delete(M, dest_id, src_id);
    assert(res == GrB_SUCCESS);

    M = _Graph_GetRelationMap(g, r);
    res = GxB_Matrix_Delete(M, dest_id, src_id);
    assert(res == GrB_SUCCESS);

    // See if source is connected to destination with additional edges.
    bool connected = false;
    int relationCount = Graph_RelationTypeCount(g);
    for(int i = 0; i < relationCount; i++) {
        M = Graph_GetRelationMatrix(g, i);
        GrB_Matrix_extractElement_BOOL(&connected, M, dest_id, src_id);
        if(connected) break;
    }

    /* There are no additional edges connecting source to destination
     * Remove edge from THE adjacency matrix. */
    if(!connected) {
        M = Graph_GetAdjacencyMatrix(g);
        res = GxB_Matrix_Delete(M, dest_id, src_id);
    }

    // Free and remove edges from datablock.
    FreeEntity(e->entity);
    DataBlock_DeleteItem(g->edges, ENTITY_GET_ID(e));
    return 1;
}

int Graph_DeleteNode(Graph *g, Node *n) {
    assert(g && n);
    
    // Check to see if indeed node exists.
    if(!DataBlock_GetItem(g->nodes, ENTITY_GET_ID(n))) return 0;

    // Delete incoming/outgoing edges of deleted node.
    Edge *e = NULL;
    Edge *edges = array_new(Edge, 32);

    Graph_GetNodeEdges(g, n, GRAPH_EDGE_DIR_BOTH, GRAPH_NO_RELATION, &edges);
    uint32_t edgeCount = array_len(edges);
    for(int j = 0; j < edgeCount; j++) Graph_DeleteEdge(g, edges+j);

    // Clear label matrix at position node ID.
    uint32_t label_count = array_len(g->labels);
    for(int i = 0; i < label_count; i++) {
        GrB_Matrix M = Graph_GetLabel(g, i);
        GxB_Matrix_Delete(M, ENTITY_GET_ID(n), ENTITY_GET_ID(n));
    }

    FreeEntity(n->entity);
    DataBlock_DeleteItem(g->nodes, ENTITY_GET_ID(n));

    // Cleanup.
    array_free(edges);
    return 1;
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

    GrB_Matrix m;
    GrB_Matrix_new(&m, GrB_BOOL, Graph_RequiredMatrixDim(g), Graph_RequiredMatrixDim(g));
    array_append(g->labels, m);
    return array_len(g->labels)-1;
}

int Graph_AddRelationType(Graph *g) {
    assert(g);

    GrB_Matrix m;
    GrB_Matrix_new(&m, GrB_BOOL, Graph_RequiredMatrixDim(g), Graph_RequiredMatrixDim(g));
    g->relations = array_append(g->relations, m);

    _Graph_AddRelationMap(g);

    // Edge mapping for relation K is at _relations_map[K].
    assert(array_len(g->_relations_map) == Graph_RelationTypeCount(g));
    int relationID = Graph_RelationTypeCount(g)-1;
    return relationID;
}

GrB_Matrix Graph_GetAdjacencyMatrix(const Graph *g) {
    assert(g);
    GrB_Matrix m = g->adjacency_matrix;
    g->SynchronizeMatrix(g, m);
    return m;
}

GrB_Matrix Graph_GetLabel(const Graph *g, int label_idx) {
    assert(g && label_idx < array_len(g->labels));
    GrB_Matrix m = g->labels[label_idx];
    g->SynchronizeMatrix(g, m);
    return m;
}

GrB_Matrix Graph_GetRelationMatrix(const Graph *g, int relation_idx) {
    assert(g && (relation_idx == GRAPH_NO_RELATION || relation_idx < Graph_RelationTypeCount(g)));
    GrB_Matrix m;

    if(relation_idx == GRAPH_NO_RELATION) {
        m = Graph_GetAdjacencyMatrix(g);
    } else {
        m = g->relations[relation_idx];
        g->SynchronizeMatrix(g, m);
    }
    return m;
}

void Graph_Free(Graph *g) {
    assert(g);
    // Free matrices.
    Entity *en;
    DataBlockIterator *it;
    GrB_Matrix m = Graph_GetAdjacencyMatrix(g);

    GrB_Matrix_free(&m);

    uint32_t relationCount = Graph_RelationTypeCount(g);
    for(int i = 0; i < relationCount; i++) {
        m = g->relations[i];
        GrB_Matrix_free(&m);
        m = g->_relations_map[i];
        GrB_Matrix_free(&m);
    }
    array_free(g->relations);
    array_free(g->_relations_map);

    uint32_t labelCount = array_len(g->labels);
    for(int i = 0; i < labelCount; i++) {
        m = g->labels[i];
        GrB_Matrix_free(&m);
    }
    array_free(g->labels);

    it = Graph_ScanNodes(g);
    while ((en = (Entity*)DataBlockIterator_Next(it)) != NULL)
        FreeEntity(en);

    DataBlockIterator_Free(it);

    it = Graph_ScanEdges(g);
    while ((en = DataBlockIterator_Next(it)) != NULL)
        FreeEntity(en);

    DataBlockIterator_Free(it);

    // Free blocks.
    DataBlock_Free(g->nodes);
    DataBlock_Free(g->edges);

    // Destroy graph-scoped locks.
    pthread_mutex_destroy(&g->_mutex);
    Graph_ReleaseLock(g);
    pthread_rwlock_destroy(&g->_rwlock);

    rm_free(g);
}
