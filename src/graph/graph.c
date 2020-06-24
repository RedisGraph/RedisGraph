/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <assert.h>

#include "graph.h"
#include "../config.h"
#include "../util/arr.h"
#include "../util/qsort.h"
#include "../GraphBLASExt/GxB_Delete.h"
#include "../util/rmalloc.h"
#include "../util/datablock/oo_datablock.h"

static GrB_BinaryOp _graph_edge_accum = NULL;
// GraphBLAS Select operator to free edge arrays and delete edges.
static GxB_SelectOp _select_delete_edges = NULL;

/* ========================= Forward declarations  ========================= */
void _MatrixResizeToCapacity(const Graph *g, RG_Matrix m);


/* ========================= GraphBLAS functions ========================= */
void _edge_accum(void *_z, const void *_x, const void *_y) {
	EdgeID *z = (EdgeID *)_z;
	const EdgeID *x = (const EdgeID *)_x;
	const EdgeID *y = (const EdgeID *)_y;

	EdgeID *ids;
	/* Single edge ID,
	 * switching from single edge ID to multiple IDs. */
	if(SINGLE_EDGE(*x)) {
		ids = array_new(EdgeID, 2);
		ids = array_append(ids, SINGLE_EDGE_ID(*x));
		ids = array_append(ids, SINGLE_EDGE_ID(*y));
		// TODO: Make sure MSB of ids isn't on.
		*z = (EdgeID)ids;
	} else {
		// Multiple edges, adding another edge.
		ids = (EdgeID *)(*x);
		ids = array_append(ids, SINGLE_EDGE_ID(*y));
		*z = (EdgeID)ids;
	}
}

/* GxB_select_function which delete edges and free edge arrays. */
bool _select_op_free_edge(GrB_Index i, GrB_Index j, GrB_Index nrows, GrB_Index ncols, const void *x,
						  const void *thunk) {
	// K is a uint64_t pointer which points to the address of our graph.
	const Graph *g = (const Graph *) * ((uint64_t *)thunk);
	const EdgeID *id = (const EdgeID *)x;
	if((SINGLE_EDGE(*id))) {
		DataBlock_DeleteItem(g->edges, SINGLE_EDGE_ID(*id));
	} else {
		/* Due to GraphBLAS V3.0+ parallelism
		 * _select_op_free_edge will be called twice
		 * Tim: "In my draft parallel GraphBLAS,
		 * most of my codes take 2 passes over the data.
		 * It's what Intel calls the Inspector / Executor model of computing.
		 * Both passes are fully parallel.
		 * The first pass is purely symbolic,
		 * and it figures out where all the output data needs to go.
		 * The 2nd pass fills in the data in the output."
		 *
		 * To avoid double freeing we'll place a marker on the first pass:
		 * ids[0] = INVALID_ENTITY_ID
		 * to be picked up by the second pass, on which the array will be freed. */
		EdgeID *ids = (EdgeID *)(*id);

		// Check for first pass marker.
		if(ids[0] != INVALID_ENTITY_ID) {
			uint id_count = array_len(ids);
			for(uint i = 0; i < id_count; i++) {
				DataBlock_DeleteItem(g->edges, ids[i]);
			}
			// Place first pass marker for second pass to pick up on.
			ids[0] = INVALID_ENTITY_ID;
		} else {
			// Second pass, simply free the array.
			array_free(ids);
		}
	}

	return false;
}

/* ========================= RG_Matrix functions =============================== */

// Creates a new matrix;
static RG_Matrix RG_Matrix_New(GrB_Type data_type, GrB_Index nrows, GrB_Index ncols) {
	RG_Matrix matrix = rm_calloc(1, sizeof(_RG_Matrix));
	GrB_Info matrix_res = GrB_Matrix_new(&matrix->grb_matrix, data_type, nrows, ncols);
	assert(matrix_res == GrB_SUCCESS);
	int mutex_res = pthread_mutex_init(&matrix->mutex, NULL);
	assert(mutex_res == 0);
	return matrix;
}

// Returns underlying GraphBLAS matrix.
static inline GrB_Matrix RG_Matrix_Get_GrB_Matrix(RG_Matrix matrix) {
	return matrix->grb_matrix;
}

// Locks the matrix.
static inline void RG_Matrix_Lock(RG_Matrix matrix) {
	pthread_mutex_lock(&matrix->mutex);
}

// Unlocks the matrix.
static inline void _RG_Matrix_Unlock(RG_Matrix matrix) {
	pthread_mutex_unlock(&matrix->mutex);
}

// Free RG_Matrix.
static void RG_Matrix_Free(RG_Matrix matrix) {
	GrB_Matrix_free(&matrix->grb_matrix);
	pthread_mutex_destroy(&matrix->mutex);
	rm_free(matrix);
}

/* ========================= Synchronization functions ========================= */

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
	/* Set _writelocked to false BEFORE unlocking
	 * if this is a reader thread no harm done,
	 * if this is a writer thread the writer is about to unlock so once again
	 * no harm done, if we set `_writelocked` to false after unlocking it is possible
	 * for a reader thread to be considered as writer, performing illegal access to
	 * underline matrices, consider a context switch after unlocking `_rwlock` but
	 * before setting `_writelocked` to false. */
	g->_writelocked = false;
	pthread_rwlock_unlock(&g->_rwlock);
}

/* Writer request access to graph. */
void Graph_WriterEnter(Graph *g) {
	pthread_mutex_lock(&g->_writers_mutex);
}

/* Writer release access to graph. */
void Graph_WriterLeave(Graph *g) {
	pthread_mutex_unlock(&g->_writers_mutex);
}

/* Force execution of all pending operations on a matrix. */
static inline void _Graph_ApplyPending(GrB_Matrix m) {
	GrB_Index nvals;
	assert(GrB_Matrix_nvals(&nvals, m) == GrB_SUCCESS);
}

/* ========================= Graph utility functions ========================= */

// Return number of nodes graph can contain.
size_t _Graph_NodeCap(const Graph *g) {
	return g->nodes->itemCap;
}

// Return number of nodes graph can contain.
size_t _Graph_EdgeCap(const Graph *g) {
	return g->edges->itemCap;
}

// Locates edges connecting src to destination.
void _Graph_GetEdgesConnectingNodes(const Graph *g, NodeID src, NodeID dest, int r, Edge **edges) {
	assert(g && src < Graph_RequiredMatrixDim(g) && dest < Graph_RequiredMatrixDim(g) &&
		   r < Graph_RelationTypeCount(g));

	Edge e;
	EdgeID edgeId;
	e.relationID = r;
	e.srcNodeID = src;
	e.destNodeID = dest;

	// relation map, maps (src, dest, r) to edge IDs.
	GrB_Matrix relation = Graph_GetRelationMatrix(g, r);
	GrB_Info res = GrB_Matrix_extractElement_UINT64(&edgeId, relation, src, dest);

	// No entry at [dest, src], src is not connected to dest with relation R.
	if(res == GrB_NO_VALUE) return;

	if(SINGLE_EDGE(edgeId)) {
		// Discard most significate bit.
		edgeId = SINGLE_EDGE_ID(edgeId);
		e.entity = DataBlock_GetItem(g->edges, edgeId);
		assert(e.entity);
		*edges = array_append(*edges, e);
	} else {
		/* Multiple edges connecting src to dest,
		 * entry is a pointer to an array of edge IDs. */
		EdgeID *edgeIds = (EdgeID *)edgeId;
		int edgeCount = array_len(edgeIds);

		for(int i = 0; i < edgeCount; i++) {
			edgeId = edgeIds[i];
			e.entity = DataBlock_GetItem(g->edges, edgeId);
			assert(e.entity);
			*edges = array_append(*edges, e);
		}
	}
}

// Tests if there's an edge of type r between src and dest nodes.
bool Graph_EdgeExists(const Graph *g, NodeID srcID, NodeID destID, int r) {
	assert(g);
	EdgeID edgeId;
	GrB_Matrix M = Graph_GetRelationMatrix(g, r);
	GrB_Info res = GrB_Matrix_extractElement_UINT64(&edgeId, M, destID, srcID);
	return res == GrB_SUCCESS;
}

static inline Entity *_Graph_GetEntity(const DataBlock *entities, EntityID id) {
	return DataBlock_GetItem(entities, id);
}

/* ============= Matrix synchronization and resizing functions =============== */

/* Resize given matrix, such that its number of row and columns
 * matches the number of nodes in the graph. Also, synchronize
 * matrix to execute any pending operations. */
void _MatrixSynchronize(const Graph *g, RG_Matrix rg_matrix) {
	GrB_Matrix m = RG_Matrix_Get_GrB_Matrix(rg_matrix);
	GrB_Index n_rows;
	GrB_Index n_cols;
	GrB_Matrix_nrows(&n_rows, m);
	GrB_Matrix_ncols(&n_cols, m);
	GrB_Index dims = Graph_RequiredMatrixDim(g);

	// If the graph belongs to one thread, we don't need to lock the mutex.
	if(g->_writelocked) {
		if((n_rows != dims) || (n_cols != dims)) {
			assert(GxB_Matrix_resize(m, dims, dims) == GrB_SUCCESS);
		}

		// Writer under write lock, no need to flush pending changes.
		return;
	}
	// Lock the matrix.
	RG_Matrix_Lock(rg_matrix);

	bool pending = false;
	GxB_Matrix_Pending(m, &pending);

	// If the matrix has pending operations or requires
	// a resize, enter critical section.
	if(pending || (n_rows != dims) || (n_cols != dims)) {
		// Double-check if resize is necessary.
		GrB_Matrix_nrows(&n_rows, m);
		GrB_Matrix_ncols(&n_cols, m);
		dims = Graph_RequiredMatrixDim(g);
		if((n_rows != dims) || (n_cols != dims)) {
			assert(GxB_Matrix_resize(m, dims, dims) == GrB_SUCCESS);
		}
		// Flush changes to matrix.
		_Graph_ApplyPending(m);
	}
	// Unlock matrix mutex.
	_RG_Matrix_Unlock(rg_matrix);
}

/* Resize matrix to node capacity. */
void _MatrixResizeToCapacity(const Graph *g, RG_Matrix matrix) {
	GrB_Matrix m = RG_Matrix_Get_GrB_Matrix(matrix);
	GrB_Index nrows;
	GrB_Index ncols;
	GrB_Matrix_ncols(&ncols, m);
	GrB_Matrix_nrows(&nrows, m);
	GrB_Index cap = _Graph_NodeCap(g);

	// This policy should only be used in a thread-safe context, so no locking is required.
	if(ncols != cap || nrows != cap) {
		assert(GxB_Matrix_resize(m, cap, cap) == GrB_SUCCESS);
	}
}

/* Do not update matrices. */
void _MatrixNOP(const Graph *g, RG_Matrix matrix) {
	return;
}

/* Define the current behavior for matrix creations and retrievals on this graph. */
void Graph_SetMatrixPolicy(Graph *g, MATRIX_POLICY policy) {
	switch(policy) {
	case SYNC_AND_MINIMIZE_SPACE:
		// Default behavior; forces execution of pending GraphBLAS operations
		// when appropriate and sizes matrices to the current node count.
		g->SynchronizeMatrix = _MatrixSynchronize;
		break;
	case RESIZE_TO_CAPACITY:
		// Bulk insertion and creation behavior; does not force pending operations
		// and resizes matrices to the graph's current node capacity.
		g->SynchronizeMatrix = _MatrixResizeToCapacity;
		break;
	case DISABLED:
		// Used when deleting or freeing a graph; forces no matrix updates or resizes.
		g->SynchronizeMatrix = _MatrixNOP;
		break;
	default:
		assert(false);
	}
}

/* Synchronize and resize all matrices in graph. */
void Graph_ApplyAllPending(Graph *g) {
	RG_Matrix M;

	for(int i = 0; i < array_len(g->labels); i ++) {
		M = g->labels[i];
		g->SynchronizeMatrix(g, M);
	}

	for(int i = 0; i < array_len(g->relations); i ++) {
		M = g->relations[i];
		g->SynchronizeMatrix(g, M);
	}

	if(Config_MaintainTranspose()) {
		for(int i = 0; i < array_len(g->t_relations); i ++) {
			M = g->t_relations[i];
			g->SynchronizeMatrix(g, M);
		}
	}
}

/* ================================ Graph API ================================ */
Graph *Graph_New(size_t node_cap, size_t edge_cap) {
	node_cap = MAX(node_cap, GRAPH_DEFAULT_NODE_CAP);
	edge_cap = MAX(node_cap, GRAPH_DEFAULT_EDGE_CAP);

	Graph *g = rm_malloc(sizeof(Graph));
	g->nodes = DataBlock_New(node_cap, sizeof(Entity), (fpDestructor)FreeEntity);
	g->edges = DataBlock_New(edge_cap, sizeof(Entity), (fpDestructor)FreeEntity);
	g->labels = array_new(RG_Matrix, GRAPH_DEFAULT_LABEL_CAP);
	g->relations = array_new(RG_Matrix, GRAPH_DEFAULT_RELATION_TYPE_CAP);
	g->adjacency_matrix = RG_Matrix_New(GrB_BOOL, node_cap, node_cap);
	g->_t_adjacency_matrix = RG_Matrix_New(GrB_BOOL, node_cap, node_cap);
	g->_zero_matrix = RG_Matrix_New(GrB_BOOL, node_cap, node_cap);
	// If we're maintaining transposed relation matrices, allocate a new array, otherwise NULL-set the pointer.
	g->t_relations = Config_MaintainTranspose() ?
					 array_new(RG_Matrix, GRAPH_DEFAULT_RELATION_TYPE_CAP) : NULL;

	// Initialize a read-write lock scoped to the individual graph
	assert(pthread_rwlock_init(&g->_rwlock, NULL) == 0);
	g->_writelocked = false;

	// Force GraphBLAS updates and resize matrices to node count by default
	Graph_SetMatrixPolicy(g, SYNC_AND_MINIMIZE_SPACE);

	// Synchronization objects initialization.
	assert(pthread_mutex_init(&g->_writers_mutex, NULL) == 0);

	// Create edge accumulator binary function
	if(!_graph_edge_accum) {
		GrB_Info info;
		info = GrB_BinaryOp_new(&_graph_edge_accum, _edge_accum, GrB_UINT64, GrB_UINT64, GrB_UINT64);
		assert(info == GrB_SUCCESS);
	}

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

uint Graph_DeletedNodeCount(const Graph *g) {
	assert(g);
	return DataBlock_DeletedItemsCount(g->nodes);
}

size_t Graph_LabeledNodeCount(const Graph *g, int label) {
	GrB_Index nvals = 0;
	GrB_Matrix m = Graph_GetLabelMatrix(g, label);
	if(m) GrB_Matrix_nvals(&nvals, m);
	return nvals;
}

size_t Graph_EdgeCount(const Graph *g) {
	assert(g);
	return g->edges->itemCount;
}

uint Graph_DeletedEdgeCount(const Graph *g) {
	assert(g);
	return DataBlock_DeletedItemsCount(g->edges);
}

int Graph_RelationTypeCount(const Graph *g) {
	return array_len(g->relations);
}

int Graph_LabelTypeCount(const Graph *g) {
	return array_len(g->labels);
}

void Graph_AllocateNodes(Graph *g, size_t n) {
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
	return (n->entity != NULL);
}

int Graph_GetEdge(const Graph *g, EdgeID id, Edge *e) {
	assert(g && id < _Graph_EdgeCap(g));
	e->entity = _Graph_GetEntity(g->edges, id);
	return (e->entity != NULL);
}

int Graph_GetNodeLabel(const Graph *g, NodeID nodeID) {
	assert(g);
	int label = GRAPH_NO_LABEL;
	for(int i = 0; i < array_len(g->labels); i++) {
		bool x = false;
		GrB_Matrix M = Graph_GetLabelMatrix(g, i);
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
	EdgeID id = ENTITY_GET_ID(e);

	// Search for relation mapping matrix M, where
	// M[dest,src] == edge ID.
	uint relationship_count = array_len(g->relations);
	for(uint i = 0; i < relationship_count; i++) {
		EdgeID edgeId = 0;
		GrB_Matrix M = Graph_GetRelationMatrix(g, i);
		GrB_Info res = GrB_Matrix_extractElement_UINT64(&edgeId, M, srcNodeID, destNodeID);
		if(res != GrB_SUCCESS) continue;

		if(SINGLE_EDGE(edgeId)) {
			EdgeID curEdgeID = SINGLE_EDGE_ID(edgeId);
			if(curEdgeID == id) {
				Edge_SetRelationID(e, i);
				return i;
			}
		} else {
			/* Multiple edges exists between src and dest
			 * see if given edge is one of them. */
			EdgeID *edges = (EdgeID *)edgeId;
			int edge_count = array_len(edges);
			for(int j = 0; j < edge_count; j++) {
				if(edges[j] == id) {
					Edge_SetRelationID(e, i);
					return i;
				}
			}
		}
	}

	// We must be able to find edge relation.
	assert(false);
	return GRAPH_NO_RELATION;
}

void Graph_GetEdgesConnectingNodes(const Graph *g, NodeID srcID, NodeID destID, int r,
								   Edge **edges) {
	assert(g && r < Graph_RelationTypeCount(g) && edges);

	// Invalid relation type specified; this can occur on multi-type traversals like:
	// MATCH ()-[:real_type|fake_type]->()
	if(r == GRAPH_UNKNOWN_RELATION) return;

	Node srcNode;
	Node destNode;
	assert(Graph_GetNode(g, srcID, &srcNode));
	assert(Graph_GetNode(g, destID, &destNode));

	if(r != GRAPH_NO_RELATION) {
		_Graph_GetEdgesConnectingNodes(g, srcID, destID, r, edges);
	} else {
		// Relation type missing, scan through each edge type.
		int relationCount = Graph_RelationTypeCount(g);
		for(int i = 0; i < relationCount; i++) {
			_Graph_GetEdgesConnectingNodes(g, srcID, destID, i, edges);
		}
	}
}

void Graph_CreateNode(Graph *g, int label, Node *n) {
	assert(g);

	NodeID id;
	Entity *en = DataBlock_AllocateItem(g->nodes, &id);
	en->id = id;
	en->prop_count = 0;
	en->properties = NULL;
	n->entity = en;

	if(label != GRAPH_NO_LABEL) {
		// Try to set matrix at position [id, id]
		// incase of a failure, scale matrix.
		RG_Matrix matrix = g->labels[label];
		GrB_Matrix m = RG_Matrix_Get_GrB_Matrix(matrix);
		GrB_Info res = GrB_Matrix_setElement_BOOL(m, true, id, id);
		if(res != GrB_SUCCESS) {
			_MatrixResizeToCapacity(g, matrix);
			assert(GrB_Matrix_setElement_BOOL(m, true, id, id) == GrB_SUCCESS);
		}
	}
}

void Graph_FormConnection(Graph *g, NodeID src, NodeID dest, EdgeID edge_id, int r) {
	GrB_Matrix adj = Graph_GetAdjacencyMatrix(g);
	GrB_Matrix tadj = Graph_GetTransposedAdjacencyMatrix(g);
	GrB_Matrix relationMat = Graph_GetRelationMatrix(g, r);

	// Rows represent source nodes, columns represent destination nodes.
	GrB_Matrix_setElement_BOOL(adj, true, src, dest);
	GrB_Matrix_setElement_BOOL(tadj, true, dest, src);
	GrB_Index I = src;
	GrB_Index J = dest;
	edge_id = SET_MSB(edge_id);
	GrB_Info info = GxB_Matrix_subassign_UINT64   // C(I,J)<Mask> = accum (C(I,J),x)
					(
						relationMat,         // input/output matrix for results
						GrB_NULL,            // optional mask for C(I,J), unused if NULL
						_graph_edge_accum,    // optional accum for Z=accum(C(I,J),x)
						edge_id,             // scalar to assign to C(I,J)
						&I,                  // row indices
						1,                   // number of row indices
						&J,                  // column indices
						1,                   // number of column indices
						GrB_NULL             // descriptor for C(I,J) and Mask
					);
	assert(info == GrB_SUCCESS);

	// Update the transposed matrix if one is present.
	if(Config_MaintainTranspose()) {
		// Perform the same update to the J,I coordinates of the transposed matrix.
		GrB_Matrix t_relationMat = Graph_GetTransposedRelationMatrix(g, r);
		GrB_Info info = GxB_Matrix_subassign_UINT64   // C(I,J)<Mask> = accum (C(I,J),x)
						(
							t_relationMat,       // input/output matrix for results
							GrB_NULL,            // optional mask for C(J,I), unused if NULL
							_graph_edge_accum,   // optional accum for Z=accum(C(J,I),x)
							edge_id,             // scalar to assign to C(J,I)
							&J,                  // row indices
							1,                   // number of row indices
							&I,                  // column indices
							1,                   // number of column indices
							GrB_NULL             // descriptor for C(J,I) and Mask
						);
		assert(info == GrB_SUCCESS);
	}
}

int Graph_ConnectNodes(Graph *g, NodeID src, NodeID dest, int r, Edge *e) {
	Node srcNode;
	Node destNode;

	assert(Graph_GetNode(g, src, &srcNode));
	assert(Graph_GetNode(g, dest, &destNode));
	assert(g && r < Graph_RelationTypeCount(g));

	EdgeID id;
	Entity *en = DataBlock_AllocateItem(g->edges, &id);
	en->id = id;
	en->prop_count = 0;
	en->properties = NULL;
	e->entity = en;
	e->relationID = r;
	e->srcNodeID = src;
	e->destNodeID = dest;
	Graph_FormConnection(g, src, dest, id, r);
	return 1;
}

/* Retrieves all either incoming or outgoing edges
 * to/from given node N, depending on given direction. */
void Graph_GetNodeEdges(const Graph *g, const Node *n, GRAPH_EDGE_DIR dir, int edgeType,
						Edge **edges) {
	assert(g && n && edges);
	GrB_Matrix M;
	NodeID srcNodeID;
	NodeID destNodeID;
	GxB_MatrixTupleIter *tupleIter;

	if(edgeType == GRAPH_UNKNOWN_RELATION) return;

	// Outgoing.
	if(dir == GRAPH_EDGE_DIR_OUTGOING || dir == GRAPH_EDGE_DIR_BOTH) {
		/* If a relationship type is specified, retrieve the appropriate relation matrix;
		 * otherwise use the overall adjacency matrix. */
		if(edgeType == GRAPH_NO_RELATION) M = Graph_GetAdjacencyMatrix(g);
		else M = Graph_GetRelationMatrix(g, edgeType);

		/* Construct an iterator to traverse the source node's row, which contains
		 * all outgoing edges. */
		GxB_MatrixTupleIter_new(&tupleIter, M);
		srcNodeID = ENTITY_GET_ID(n);
		GxB_MatrixTupleIter_iterate_row(tupleIter, srcNodeID);
		while(true) {
			bool depleted = false;
			GxB_MatrixTupleIter_next(tupleIter, NULL, &destNodeID, &depleted);
			if(depleted) break;
			// Collect all edges connecting this source node to each of its destinations.
			Graph_GetEdgesConnectingNodes(g, srcNodeID, destNodeID, edgeType, edges);
		}
		GxB_MatrixTupleIter_free(tupleIter);
	}

	// Incoming.
	if(dir == GRAPH_EDGE_DIR_INCOMING || dir == GRAPH_EDGE_DIR_BOTH) {
		/* Retrieve the transposed adjacency matrix, regardless of whether or not
		 * a relationship type is specified. */
		M = Graph_GetTransposedAdjacencyMatrix(g);

		/* Construct an iterator to traverse the node's row, which in the transposed
		 * adjacency matrix contains all incoming edges. */
		GxB_MatrixTupleIter_new(&tupleIter, M);
		destNodeID = ENTITY_GET_ID(n);
		GxB_MatrixTupleIter_iterate_row(tupleIter, destNodeID);

		while(true) {
			bool depleted = false;
			GxB_MatrixTupleIter_next(tupleIter, NULL, &srcNodeID, &depleted);
			if(depleted) break;
			/* Collect all edges connecting this destination node to each of its sources.
			 * This call will only collect edges of the appropriate relationship type,
			 * if one is specified. */
			Graph_GetEdgesConnectingNodes(g, srcNodeID, destNodeID, edgeType, edges);
		}

		// Clean up
		GxB_MatrixTupleIter_free(tupleIter);
	}
}

/* Removes an edge from Graph and updates graph relevent matrices. */
int Graph_DeleteEdge(Graph *g, Edge *e) {
	uint64_t x;
	GrB_Matrix R;
	GrB_Matrix M;
	GrB_Info info;
	EdgeID edge_id;
	GrB_Matrix TR = GrB_NULL;
	int r = Edge_GetRelationID(e);
	NodeID src_id = Edge_GetSrcNodeID(e);
	NodeID dest_id = Edge_GetDestNodeID(e);

	R = Graph_GetRelationMatrix(g, r);
	if(Config_MaintainTranspose()) TR = Graph_GetTransposedRelationMatrix(g, r);

	// Test to see if edge exists.
	info = GrB_Matrix_extractElement(&edge_id, R, src_id, dest_id);
	if(info != GrB_SUCCESS) return 0;

	if(SINGLE_EDGE(edge_id)) {
		// Single edge of type R connecting src to dest, delete entry.
		assert(GxB_Matrix_Delete(R, src_id, dest_id) == GrB_SUCCESS);
		if(TR) assert(GxB_Matrix_Delete(TR, dest_id, src_id) == GrB_SUCCESS);

		// See if source is connected to destination with additional edges.
		bool connected = false;
		int relationCount = Graph_RelationTypeCount(g);
		for(int i = 0; i < relationCount; i++) {
			if(i == r) continue;
			M = Graph_GetRelationMatrix(g, i);
			info = GrB_Matrix_extractElement(&x, M, src_id, dest_id);
			if(info == GrB_SUCCESS) {
				connected = true;
				break;
			}
		}

		/* There are no additional edges connecting source to destination
		 * Remove edge from THE adjacency matrix. */
		if(!connected) {
			M = Graph_GetAdjacencyMatrix(g);
			assert(GxB_Matrix_Delete(M, src_id, dest_id) == GrB_SUCCESS);

			M = Graph_GetTransposedAdjacencyMatrix(g);
			assert(GxB_Matrix_Delete(M, dest_id, src_id) == GrB_SUCCESS);
		}
	} else {
		/* Multiple edges connecting src to dest
		 * locate specific edge and remove it
		 * revert back from array representation to edge ID
		 * incase we're left with a single edge connecting src to dest. */

		int i = 0;
		EdgeID id = ENTITY_GET_ID(e);
		EdgeID *edges = (EdgeID *)edge_id;
		int edge_count = array_len(edges);

		// Locate edge within edge array.
		for(; i < edge_count; i++) if(edges[i] == id) break;
		assert(i < edge_count);

		/* Remove edge from edge array
		 * migrate last edge ID and reduce array size.
		 * TODO: reallocate array of size / capacity ratio is high. */
		edges[i] = edges[edge_count - 1];
		array_pop(edges);

		/* Incase we're left with a single edge connecting src to dest
		 * revert back from array to scalar. */
		if(array_len(edges) == 1) {
			edge_id = edges[0];
			array_free(edges);
			GrB_Matrix_setElement(R, SET_MSB(edge_id), src_id, dest_id);
		}

		if(TR) {
			/* We must make the matching updates to the transposed matrix.
			 * First, extract the element that is known to be an edge array. */
			info = GrB_Matrix_extractElement(edges, TR, dest_id, src_id);
			assert(info == GrB_SUCCESS);
			// Replace the deleted edge with the last edge in the matrix.
			edges[i] = edges[edge_count - 1];
			array_pop(edges);
			// Free and replace the array if it now has 1 element.
			if(array_len(edges) == 1) {
				edge_id = edges[0];
				array_free(edges);
				GrB_Matrix_setElement(TR, SET_MSB(edge_id), dest_id, src_id);
			}
		}
	}

	// Free and remove edges from datablock.
	DataBlock_DeleteItem(g->edges, ENTITY_GET_ID(e));
	return 1;
}

void Graph_DeleteNode(Graph *g, Node *n) {
	/* Assumption, node is completely detected,
	 * there are no incoming nor outgoing edges
	 * leading to / from node. */
	assert(g && n);

	// Clear label matrix at position node ID.
	uint32_t label_count = array_len(g->labels);
	for(int i = 0; i < label_count; i++) {
		GrB_Matrix M = Graph_GetLabelMatrix(g, i);
		GxB_Matrix_Delete(M, ENTITY_GET_ID(n), ENTITY_GET_ID(n));
	}

	DataBlock_DeleteItem(g->nodes, ENTITY_GET_ID(n));
}

static void _Graph_FreeRelationMatrices(Graph *g) {
	if(!_select_delete_edges) {
		// The select operator has not yet been constructed; build it now.
		GrB_Info res;
		res = GxB_SelectOp_new(&_select_delete_edges, _select_op_free_edge, GrB_UINT64, GrB_UINT64);
		assert(res == GrB_SUCCESS);
	}

	GxB_Scalar thunk;
	GxB_Scalar_new(&thunk, GrB_UINT64);
	GxB_Scalar_setElement_UINT64(thunk, (uint64_t)g);

	uint relationCount = Graph_RelationTypeCount(g);
	for(uint i = 0; i < relationCount; i++) {
		RG_Matrix M = g->relations[i];
		// Use the edge deletion Select operator to free all edge arrays within the adjacency matrix.
		GxB_select(M->grb_matrix, GrB_NULL, GrB_NULL, _select_delete_edges, M->grb_matrix, thunk, GrB_NULL);

		// Free the matrix itself.
		RG_Matrix_Free(M);

		// Perform the same update to transposed matrices.
		if(Config_MaintainTranspose()) {
			RG_Matrix TM = g->t_relations[i];
			GxB_select(TM->grb_matrix, GrB_NULL, GrB_NULL, _select_delete_edges, TM->grb_matrix, thunk,
					   GrB_NULL);
			// Free the matrix itself.
			RG_Matrix_Free(TM);
		}
	}

	GrB_free(&thunk);
}

static void _BulkDeleteNodes(Graph *g, Node *nodes, uint node_count,
							 uint *node_deleted, uint *edge_deleted) {
	assert(g && g->_writelocked && nodes && node_count > 0);

	if(!_select_delete_edges) {
		// The select operator has not yet been constructed; build it now.
		GrB_Info res;
		res = GxB_SelectOp_new(&_select_delete_edges, _select_op_free_edge, GrB_UINT64, GrB_UINT64);
		assert(res == GrB_SUCCESS);
	}

	/* Create a matrix M where M[j,i] = 1 where:
	 * Node i is connected to node j. */

	GrB_Matrix A;                       // A = R(M) masked relation matrix.
	GrB_Index nvals;                    // Number of elements in mask.
	GrB_Matrix Mask;                    // Mask noteing all implicitly deleted edges.
	GrB_Matrix Nodes;                   // Mask noteing each node marked for deletion.
	GrB_Matrix adj;                     // Adjacency matrix.
	GrB_Matrix tadj;                    // Transposed adjacency matrix.
	GrB_Descriptor desc;                // GraphBLAS descriptor.
	GxB_MatrixTupleIter *adj_iter;      // iterator over the adjacency matrix.
	GxB_MatrixTupleIter *tadj_iter;     // iterator over the transposed adjacency matrix.

	GrB_Descriptor_new(&desc);
	adj = Graph_GetAdjacencyMatrix(g);
	tadj = Graph_GetTransposedAdjacencyMatrix(g);
	GxB_MatrixTupleIter_new(&adj_iter, adj);
	GxB_MatrixTupleIter_new(&tadj_iter, tadj);
	GrB_Matrix_new(&A, GrB_UINT64, Graph_RequiredMatrixDim(g), Graph_RequiredMatrixDim(g));
	GrB_Matrix_new(&Mask, GrB_BOOL, Graph_RequiredMatrixDim(g), Graph_RequiredMatrixDim(g));
	GrB_Matrix_new(&Nodes, GrB_BOOL, Graph_RequiredMatrixDim(g), Graph_RequiredMatrixDim(g));

	/* For user-defined select operators,
	 * If Thunk is not NULL, it must be a valid GxB_Scalar. If it has no entry,
	 * it is treated as if it had a single entry equal to zero, for built-in types (not
	 * user-defined types).
	 * For user-defined select operators, the entry is passed to the user-defined
	 * select operator, with no typecasting. Its type must be identical to 'ttype' of
	 * the select operator. */
	GxB_Scalar thunk;
	GxB_Scalar_new(&thunk, GrB_UINT64);
	GxB_Scalar_setElement_UINT64(thunk, (uint64_t)g);

	// Populate mask with implicit edges, take note of deleted nodes.
	for(uint i = 0; i < node_count; i++) {
		GrB_Index src;
		GrB_Index dest;
		Node *n = nodes + i;
		bool depleted = false;
		NodeID ID = ENTITY_GET_ID(n);

		// Outgoing edges.
		GxB_MatrixTupleIter_iterate_row(adj_iter, ID);
		while(true) {
			GxB_MatrixTupleIter_next(adj_iter, NULL,  &dest, &depleted);
			if(depleted) break;
			GrB_Matrix_setElement_BOOL(Mask, true, ID, dest);
		}

		depleted = false;

		// Incoming edges.
		GxB_MatrixTupleIter_iterate_row(tadj_iter, ID);
		while(true) {
			GxB_MatrixTupleIter_next(tadj_iter, NULL, &src, &depleted);
			if(depleted) break;
			GrB_Matrix_setElement_BOOL(Mask, true, src, ID);
		}

		GrB_Matrix_setElement_BOOL(Nodes, true, ID, ID);
	}

	GrB_Matrix_nvals(&nvals, Nodes);
	*node_deleted += nvals;

	GrB_Matrix_nvals(&nvals, Mask);
	*edge_deleted += nvals;

	// Clear updated output matrix before assignment.
	GrB_Descriptor_set(desc, GrB_OUTP, GrB_REPLACE);

	// Free and remove implicit edges from relation matrices.
	int relation_count = Graph_RelationTypeCount(g);
	for(int i = 0; i < relation_count; i++) {
		GrB_Matrix R = Graph_GetRelationMatrix(g, i);

		// Reset mask descriptor.
		GrB_Descriptor_set(desc, GrB_MASK, GxB_DEFAULT);

		/* Isolate implicit edges.
		 * A will contain all implicitly deleted edges from R. */
		GrB_Matrix_apply(A, Mask, GrB_NULL, GrB_IDENTITY_UINT64, R, desc);

		/* Free each multi edge array entry in A
		 * Call _select_op_free_edge on each entry of A. */
		GxB_select(A, GrB_NULL, GrB_NULL, _select_delete_edges, A, thunk, GrB_NULL);

		// Clear the relation matrix.
		GrB_Descriptor_set(desc, GrB_MASK, GrB_COMP);

		// Remove every entry of R marked by Mask.
		GrB_Matrix_apply(R, Mask, GrB_NULL, GrB_IDENTITY_UINT64, R, desc);
	}

	/* Descriptor:
	 * GrB_MASK, GrB_COMP */
	GrB_Descriptor_set(desc, GrB_MASK, GrB_COMP);
	// Update the adjacency matrix to remove deleted entries.
	GrB_Matrix_apply(adj, Mask, GrB_NULL, GrB_IDENTITY_BOOL, adj, desc);

	// Transpose the mask so that it will match the transposed adjacency matrix.
	GrB_transpose(Mask, GrB_NULL,  GrB_NULL, Mask, GrB_NULL);

	// Update the transposed adjacency matrix.
	GrB_Matrix_apply(tadj, Mask, GrB_NULL, GrB_IDENTITY_BOOL, tadj, desc);

	/* If we have individual transposed matrices, repeat all the above steps
	 * with the transposed Mask. */
	if(Config_MaintainTranspose()) {
		for(int i = 0; i < relation_count; i++) {
			GrB_Matrix TR = Graph_GetTransposedRelationMatrix(g, i);

			// Reset mask descriptor.
			GrB_Descriptor_set(desc, GrB_MASK, GxB_DEFAULT);

			/* Isolate implicit edges.
			 * A will contain all implicitly deleted edges from TR. */
			GrB_Matrix_apply(A, Mask, GrB_NULL, GrB_IDENTITY_UINT64, TR, desc);

			/* Free each multi edge array entry in A
			 * Call _select_op_free_edge on each entry of A. */
			GxB_select(A, GrB_NULL, GrB_NULL, _select_delete_edges, A, thunk, GrB_NULL);

			// Clear the relation matrix.
			GrB_Descriptor_set(desc, GrB_MASK, GrB_COMP);

			// Remove every entry of TR marked by Mask.
			GrB_Matrix_apply(TR, Mask, GrB_NULL, GrB_IDENTITY_UINT64, TR, desc);
		}
	}

	/* Delete nodes
	 * All nodes marked for deleteion are detected, no incoming / outgoing edges. */
	int node_type_count = Graph_LabelTypeCount(g);
	for(int i = 0; i < node_type_count; i++) {
		GrB_Matrix L = Graph_GetLabelMatrix(g, i);
		GrB_Matrix_apply(L, Nodes, GrB_NULL, GrB_IDENTITY_BOOL, L, desc);
	}

	for(uint i = 0; i < node_count; i++) {
		Node *n = nodes + i;
		DataBlock_DeleteItem(g->nodes, ENTITY_GET_ID(n));
	}

	// Clean up.
	GrB_free(&A);
	GrB_free(&desc);
	GrB_free(&Mask);
	GrB_free(&thunk);
	GrB_free(&Nodes);
	GxB_MatrixTupleIter_free(adj_iter);
	GxB_MatrixTupleIter_free(tadj_iter);
}

static void _BulkDeleteEdges(Graph *g, Edge *edges, size_t edge_count) {
	assert(g && g->_writelocked && edges && edge_count > 0);

	int relationCount = Graph_RelationTypeCount(g);
	GrB_Matrix masks[relationCount];
	for(int i = 0; i < relationCount; i++) masks[i] = NULL;
	bool update_adj_matrices = false;

	for(int i = 0; i < edge_count; i++) {
		Edge *e = edges + i;
		int r = Edge_GetRelationID(e);
		NodeID src_id = Edge_GetSrcNodeID(e);
		NodeID dest_id = Edge_GetDestNodeID(e);
		EdgeID edge_id;
		GrB_Matrix R = Graph_GetRelationMatrix(g, r);  // Relation matrix.
		GrB_Matrix TR = Config_MaintainTranspose() ? Graph_GetTransposedRelationMatrix(g, r) : NULL;
		GrB_Matrix_extractElement(&edge_id, R, src_id, dest_id);

		if(SINGLE_EDGE(edge_id)) {
			update_adj_matrices = true;
			GrB_Matrix mask = masks[r];    // mask noteing all deleted edges.
			// Get mask of this relation type.
			if(mask == NULL) {
				GrB_Matrix_new(&mask, GrB_BOOL, Graph_RequiredMatrixDim(g), Graph_RequiredMatrixDim(g));
				masks[r] = mask;
			}
			// Update mask.
			GrB_Matrix_setElement_BOOL(mask, true, src_id, dest_id);
		} else {
			/* Multiple edges connecting src to dest
			 * locate specific edge and remove it
			 * revert back from array representation to edge ID
			 * incase we're left with a single edge connecting src to dest. */

			int i = 0;
			EdgeID id = ENTITY_GET_ID(e);
			EdgeID *multi_edges = (EdgeID *)edge_id;
			int multi_edge_count = array_len(multi_edges);

			// Locate edge within edge array.
			for(; i < multi_edge_count; i++) if(multi_edges[i] == id) break;
			assert(i < multi_edge_count);

			/* Remove edge from edge array
			 * migrate last edge ID and reduce array size.
			 * TODO: reallocate array of size / capacity ratio is high. */
			multi_edges[i] = multi_edges[multi_edge_count - 1];
			array_pop(multi_edges);

			/* Incase we're left with a single edge connecting src to dest
			 * revert back from array to scalar. */
			if(array_len(multi_edges) == 1) {
				edge_id = multi_edges[0];
				array_free(multi_edges);
				GrB_Matrix_setElement(R, SET_MSB(edge_id), src_id, dest_id);
			}

			if(TR) {
				/* We must make the matching updates to the transposed matrix.
				 * First, extract the element that is known to be an edge array. */
				GrB_Matrix_extractElement(&edge_id, TR, dest_id, src_id);
				multi_edges = (EdgeID *)edge_id;
				int multi_edge_count = array_len(multi_edges);
				multi_edges[i] = multi_edges[multi_edge_count - 1];
				array_pop(multi_edges);
				// Free and replace the array if it now has 1 element.
				if(array_len(multi_edges) == 1) {
					edge_id = multi_edges[0];
					array_free(multi_edges);
					GrB_Matrix_setElement(TR, SET_MSB(edge_id), dest_id, src_id);
				}
			}
		}

		// Free and remove edges from datablock.
		DataBlock_DeleteItem(g->edges, ENTITY_GET_ID(e));
	}

	if(update_adj_matrices) {
		GrB_Matrix remaining_mask;
		GrB_Matrix_new(&remaining_mask, GrB_BOOL, Graph_RequiredMatrixDim(g), Graph_RequiredMatrixDim(g));
		GrB_Descriptor desc;    // GraphBLAS descriptor.
		GrB_Descriptor_new(&desc);
		// Descriptor sets to clear entry according to mask.
		GrB_Descriptor_set(desc, GrB_MASK, GrB_COMP);

		// Clear updated output matrix before assignment.
		GrB_Descriptor_set(desc, GrB_OUTP, GrB_REPLACE);

		for(int r = 0; r < relationCount; r++) {
			GrB_Matrix mask = masks[r];
			GrB_Matrix R = Graph_GetRelationMatrix(g, r);  // Relation matrix.
			if(mask) {
				// Remove every entry of R marked by Mask.
				// Desc: GrB_MASK = GrB_COMP,  GrB_OUTP = GrB_REPLACE.
				// R = R & !mask.
				GrB_Matrix_apply(R, mask, GrB_NULL, GrB_IDENTITY_UINT64, R, desc);
				if(Config_MaintainTranspose()) {
					GrB_Matrix tM = Graph_GetTransposedRelationMatrix(g, r);  // Transposed relation mapping matrix.
					// Transpose mask (this cannot be done by descriptor).
					GrB_transpose(mask, GrB_NULL, GrB_NULL, mask, GrB_NULL);
					// tM = tM & !mask.
					GrB_Matrix_apply(tM, mask, GrB_NULL, GrB_IDENTITY_UINT64, tM, desc);
				}
				GrB_free(&mask);
			}

			// Collect remaining edges. remaining_mask = remaining_mask + R.
			GrB_eWiseAdd_Matrix_Semiring(remaining_mask, GrB_NULL, GrB_NULL, GxB_ANY_PAIR_BOOL, remaining_mask,
										 R, GrB_NULL);
		}

		GrB_Matrix adj_matrix = Graph_GetAdjacencyMatrix(g);
		GrB_Matrix t_adj_matrix = Graph_GetTransposedAdjacencyMatrix(g);
		// To calculate edges to delete, remove all the remaining edges from "The" adjency matrix.
		// Set descriptor mask to default.
		GrB_Descriptor_set(desc, GrB_MASK, GxB_DEFAULT);
		// adj_matrix = adj_matrix & remaining_mask.
		GrB_Matrix_apply(adj_matrix, remaining_mask, GrB_NULL, GrB_IDENTITY_BOOL, adj_matrix, desc);
		// Transpose remaining_mask.
		GrB_transpose(remaining_mask, GrB_NULL,  GrB_NULL, remaining_mask, GrB_NULL);
		// t_adj_matrix = t_adj_matrix & remaining_mask.
		GrB_Matrix_apply(t_adj_matrix, remaining_mask, GrB_NULL, GrB_IDENTITY_BOOL, t_adj_matrix, desc);

		GrB_free(&remaining_mask);
		GrB_free(&desc);
	}
}

/* Removes both nodes and edges from graph. */
void Graph_BulkDelete(Graph *g, Node *nodes, uint node_count, Edge *edges, uint edge_count,
					  uint *node_deleted, uint *edge_deleted) {
	assert(g);

	*edge_deleted = 0;
	*node_deleted = 0;

	if(node_count) _BulkDeleteNodes(g, nodes, node_count, node_deleted, edge_deleted);

	if(edge_count) {
		// Filter out explicit edges which were removed by _BulkDeleteNodes.
		if(node_count) {
			for(int i = 0; i < edge_count; i++) {
				Edge *e = edges + i;
				NodeID src = Edge_GetSrcNodeID(e);
				NodeID dest = Edge_GetDestNodeID(e);

				if(!DataBlock_GetItem(g->nodes, src) || !DataBlock_GetItem(g->nodes, dest)) {
					/* Edge already removed due to node removal.
					* Replace current edge with last edge. */
					edges[i] = edges[edge_count - 1];

					// Update indices.
					i--;
					edge_count--;
				}
			}
		}

		/* it might be that edge_count dropped to 0
		 * due to implicit edge deletion. */
		if(edge_count == 0) return;

		// Removing duplicates.
#define is_edge_lt(a, b) (ENTITY_GET_ID((a)) < ENTITY_GET_ID((b)))
		QSORT(Edge, edges, edge_count, is_edge_lt);

		size_t uniqueIdx = 0;
		for(int i = 0; i < edge_count; i++) {
			// As long as current is the same as follows.
			while(i < edge_count - 1 && ENTITY_GET_ID(edges + i) == ENTITY_GET_ID(edges + i + 1)) i++;

			if(uniqueIdx < i) edges[uniqueIdx] = edges[i];
			uniqueIdx++;
		}

		edge_count = uniqueIdx;
		_BulkDeleteEdges(g, edges, edge_count);
	}

	*edge_deleted += edge_count;
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
	RG_Matrix m = RG_Matrix_New(GrB_BOOL, Graph_RequiredMatrixDim(g), Graph_RequiredMatrixDim(g));
	array_append(g->labels, m);
	return array_len(g->labels) - 1;
}

int Graph_AddRelationType(Graph *g) {
	assert(g);

	size_t dims = Graph_RequiredMatrixDim(g);
	RG_Matrix m = RG_Matrix_New(GrB_UINT64, dims, dims);
	g->relations = array_append(g->relations, m);
	if(Config_MaintainTranspose()) {
		RG_Matrix tm = RG_Matrix_New(GrB_UINT64, dims, dims);
		g->t_relations = array_append(g->t_relations, tm);
	}

	int relationID = Graph_RelationTypeCount(g) - 1;
	return relationID;
}

GrB_Matrix Graph_GetAdjacencyMatrix(const Graph *g) {
	assert(g);
	RG_Matrix m = g->adjacency_matrix;
	g->SynchronizeMatrix(g, m);
	return RG_Matrix_Get_GrB_Matrix(m);
}

// Get the transposed adjacency matrix.
GrB_Matrix Graph_GetTransposedAdjacencyMatrix(const Graph *g) {
	assert(g);
	RG_Matrix m = g->_t_adjacency_matrix;
	g->SynchronizeMatrix(g, m);
	return RG_Matrix_Get_GrB_Matrix(m);
}

GrB_Matrix Graph_GetLabelMatrix(const Graph *g, int label_idx) {
	assert(g && label_idx < array_len(g->labels));
	RG_Matrix m = g->labels[label_idx];
	g->SynchronizeMatrix(g, m);
	return RG_Matrix_Get_GrB_Matrix(m);
}

GrB_Matrix Graph_GetRelationMatrix(const Graph *g, int relation_idx) {
	assert(g && (relation_idx == GRAPH_NO_RELATION || relation_idx < Graph_RelationTypeCount(g)));

	if(relation_idx == GRAPH_NO_RELATION) {
		return Graph_GetAdjacencyMatrix(g);
	} else {
		RG_Matrix m = g->relations[relation_idx];
		g->SynchronizeMatrix(g, m);
		return RG_Matrix_Get_GrB_Matrix(m);
	}
}

GrB_Matrix Graph_GetTransposedRelationMatrix(const Graph *g, int relation_idx) {
	assert(g && (relation_idx == GRAPH_NO_RELATION || relation_idx < Graph_RelationTypeCount(g)));

	if(relation_idx == GRAPH_NO_RELATION) {
		return Graph_GetTransposedAdjacencyMatrix(g);
	} else {
		assert(g->t_relations && "tried to retrieve nonexistent transposed matrix.");

		RG_Matrix m = g->t_relations[relation_idx];
		g->SynchronizeMatrix(g, m);
		return RG_Matrix_Get_GrB_Matrix(m);
	}
}

GrB_Matrix Graph_GetZeroMatrix(const Graph *g) {
	GrB_Index nvals;
	RG_Matrix z = g->_zero_matrix;
	g->SynchronizeMatrix(g, z);

	// Make sure zero matrix is indeed empty.
	GrB_Matrix grb_z = RG_Matrix_Get_GrB_Matrix(z);
	GrB_Matrix_nvals(&nvals, grb_z);
	assert(nvals == 0);
	return grb_z;
}

void Graph_Free(Graph *g) {
	assert(g);
	// Free matrices.
	Entity *en;
	DataBlockIterator *it;
	RG_Matrix_Free(g->_zero_matrix);
	RG_Matrix_Free(g->adjacency_matrix);
	RG_Matrix_Free(g->_t_adjacency_matrix);

	_Graph_FreeRelationMatrices(g);
	array_free(g->relations);
	array_free(g->t_relations);

	uint32_t labelCount = array_len(g->labels);
	for(int i = 0; i < labelCount; i++) {
		RG_Matrix_Free(g->labels[i]);
	}
	array_free(g->labels);

	it = Graph_ScanNodes(g);
	while((en = (Entity *)DataBlockIterator_Next(it)) != NULL)
		FreeEntity(en);

	DataBlockIterator_Free(it);

	it = Graph_ScanEdges(g);
	while((en = DataBlockIterator_Next(it)) != NULL)
		FreeEntity(en);

	DataBlockIterator_Free(it);

	// Free blocks.
	DataBlock_Free(g->nodes);
	DataBlock_Free(g->edges);

	assert(pthread_mutex_destroy(&g->_writers_mutex) == 0);

	if(g->_writelocked) Graph_ReleaseLock(g);
	assert(pthread_rwlock_destroy(&g->_rwlock) == 0);

	rm_free(g);
}

