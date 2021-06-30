/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "graph.h"
#include "RG.h"
#include "../util/arr.h"
#include "../util/qsort.h"
#include "../util/rmalloc.h"
#include "../configuration/config.h"
#include "../GraphBLASExt/GxB_Delete.h"
#include "../util/datablock/oo_datablock.h"

static GrB_BinaryOp _graph_edge_accum = NULL;
// GraphBLAS binary operator for freeing edges
static GrB_BinaryOp _binary_op_delete_edges = NULL;

//------------------------------------------------------------------------------
// Forward declarations
//------------------------------------------------------------------------------
void _MatrixResizeToCapacity(const Graph *g, RG_Matrix m);

//------------------------------------------------------------------------------
// GraphBLAS functions
//------------------------------------------------------------------------------
void _edge_accum(void *_z, const void *_x, const void *_y) {
	EdgeID *z = (EdgeID *)_z;
	const EdgeID *x = (const EdgeID *)_x;
	const EdgeID *y = (const EdgeID *)_y;

	EdgeID *ids;
	/* Single edge ID,
	 * switching from single edge ID to multiple IDs. */
	if(SINGLE_EDGE(*x)) {
		ids = array_new(EdgeID, 2);
		array_append(ids, SINGLE_EDGE_ID(*x));
		array_append(ids, SINGLE_EDGE_ID(*y));
		// TODO: Make sure MSB of ids isn't on.
		*z = (EdgeID)ids;
	} else {
		// Multiple edges, adding another edge.
		ids = (EdgeID *)(*x);
		array_append(ids, SINGLE_EDGE_ID(*y));
		*z = (EdgeID)ids;
	}
}

void _binary_op_free_edge(void *z, const void *x, const void *y) {
	const Graph *g = (const Graph *) * ((uint64_t *)x);
	const EdgeID *id = (const EdgeID *)y;

	if((SINGLE_EDGE(*id))) {
		DataBlock_DeleteItem(g->edges, SINGLE_EDGE_ID(*id));
	} else {
		EdgeID *ids = (EdgeID *)(*id);
		uint id_count = array_len(ids);
		for(uint i = 0; i < id_count; i++) {
			DataBlock_DeleteItem(g->edges, ids[i]);
		}
		array_free(ids);
	}
}

/* ========================= RG_Matrix functions =============================== */

// Creates a new matrix
static RG_Matrix RG_Matrix_New(const Graph *g, GrB_Type data_type) {
	RG_Matrix matrix = rm_calloc(1, sizeof(_RG_Matrix));

	matrix->dirty = true;
	matrix->allow_multi_edge = true;

	GrB_Index n = Graph_RequiredMatrixDim(g);
	GrB_Info matrix_res = GrB_Matrix_new(&matrix->grb_matrix, data_type, n, n);
	ASSERT(matrix_res == GrB_SUCCESS);

	int mutex_res = pthread_mutex_init(&matrix->mutex, NULL);
	ASSERT(mutex_res == 0);

	return matrix;
}

// Returns underlying GraphBLAS matrix.
static inline GrB_Matrix RG_Matrix_Get_GrB_Matrix(RG_Matrix matrix) {
	return matrix->grb_matrix;
}

static inline bool RG_Matrix_IsDirty(RG_Matrix matrix) {
	return matrix->dirty;
}

static inline void RG_Matrix_SetDirty(RG_Matrix matrix) {
	matrix->dirty = true;
}

static inline void RG_Matrix_SetUnDirty(RG_Matrix matrix) {
	matrix->dirty = false;
}

// Locks the matrix.
static inline void RG_Matrix_Lock(RG_Matrix matrix) {
	pthread_mutex_lock(&matrix->mutex);
}

// Unlocks the matrix.
static inline void _RG_Matrix_Unlock(RG_Matrix matrix) {
	pthread_mutex_unlock(&matrix->mutex);
}

static inline bool _RG_Matrix_MultiEdgeEnabled(const RG_Matrix matrix) {
	return matrix->allow_multi_edge;
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
	GrB_Info res = GrB_wait(&m);
	ASSERT(res == GrB_SUCCESS);
}

/* ========================= Graph utility functions ========================= */

// Return number of nodes graph can contain.
static inline size_t _Graph_NodeCap(const Graph *g) {
	return g->nodes->itemCap;
}

// Return number of nodes graph can contain.
static inline size_t _Graph_EdgeCap(const Graph *g) {
	return g->edges->itemCap;
}

// Locates edges connecting src to destination.
void _Graph_GetEdgesConnectingNodes(const Graph *g, NodeID src, NodeID dest, int r, Edge **edges) {
	ASSERT(g && src < Graph_RequiredMatrixDim(g) && dest < Graph_RequiredMatrixDim(g) &&
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
		e.id = edgeId;
		ASSERT(e.entity);
		array_append(*edges, e);
	} else {
		/* Multiple edges connecting src to dest,
		 * entry is a pointer to an array of edge IDs. */
		EdgeID *edgeIds = (EdgeID *)edgeId;
		int edgeCount = array_len(edgeIds);

		for(int i = 0; i < edgeCount; i++) {
			edgeId = edgeIds[i];
			e.entity = DataBlock_GetItem(g->edges, edgeId);
			e.id = edgeId;
			ASSERT(e.entity);
			array_append(*edges, e);
		}
	}
}

static inline Entity *_Graph_GetEntity(const DataBlock *entities, EntityID id) {
	return DataBlock_GetItem(entities, id);
}

/* ============= Matrix synchronization and resizing functions =============== */

static inline void _Graph_SetAdjacencyMatrixDirty(const Graph *g) {
	RG_Matrix_SetDirty(g->adjacency_matrix);
	RG_Matrix_SetDirty(g->_t_adjacency_matrix);
}

static inline void _Graph_SetLabelMatrixDirty(const Graph *g, int label_idx) {
	ASSERT(g && label_idx < array_len(g->labels));
	RG_Matrix_SetDirty(g->labels[label_idx]);
}

static inline void _Graph_SetRelationMatrixDirty(const Graph *g, int relation_idx) {
	ASSERT(g && (relation_idx == GRAPH_NO_RELATION || relation_idx < Graph_RelationTypeCount(g)));
	if(relation_idx == GRAPH_NO_RELATION) {
		_Graph_SetAdjacencyMatrixDirty(g);
	} else {
		RG_Matrix_SetDirty(g->relations[relation_idx]);
		bool maintain_transpose;
		Config_Option_get(Config_MAINTAIN_TRANSPOSE, &maintain_transpose);
		if(maintain_transpose) RG_Matrix_SetDirty(g->t_relations[relation_idx]);
	}
}

/* Resize given matrix, such that its number of row and columns
 * matches the number of nodes in the graph. Also, synchronize
 * matrix to execute any pending operations. */
void _MatrixSynchronize(const Graph *g, RG_Matrix rg_matrix) {
	bool dirty = RG_Matrix_IsDirty(rg_matrix);
	GrB_Matrix m = RG_Matrix_Get_GrB_Matrix(rg_matrix);
	GrB_Index n_rows;
	GrB_Index n_cols;
	GrB_Matrix_nrows(&n_rows, m);
	GrB_Matrix_ncols(&n_cols, m);
	GrB_Index dims = Graph_RequiredMatrixDim(g);

	// matrix must be resized if its dimensions missmatch required dimensions
	bool require_resize = (n_rows != dims || n_cols != dims);

	// matrix fully synced, nothing to do
	if(!require_resize && !RG_Matrix_IsDirty(rg_matrix)) return;

	//--------------------------------------------------------------------------
	// sync under WRITE
	//--------------------------------------------------------------------------

	// if the graph belongs to one thread, we don't need to lock the mutex
	if(g->_writelocked) {
		if(require_resize) {
			GrB_Info res = GxB_Matrix_resize(m, dims, dims);
			ASSERT(res == GrB_SUCCESS);
		}

		// writer under write lock, no need to flush pending changes
		return;
	}

	//--------------------------------------------------------------------------
	// sync under READ
	//--------------------------------------------------------------------------

	// lock the matrix
	RG_Matrix_Lock(rg_matrix);

	// recheck
	GrB_Matrix_nrows(&n_rows, m);
	GrB_Matrix_ncols(&n_cols, m);
	dims = Graph_RequiredMatrixDim(g);
	require_resize = (n_rows != dims || n_cols != dims);

	// some other thread performed sync
	if(!require_resize && !RG_Matrix_IsDirty(rg_matrix)) goto cleanup;

	// resize if required
	if(require_resize) {
		GrB_Info res = GxB_Matrix_resize(m, dims, dims);
		ASSERT(res == GrB_SUCCESS);
	}

	// flush pending changes if dirty
	if(RG_Matrix_IsDirty(rg_matrix)) _Graph_ApplyPending(m);

	RG_Matrix_SetUnDirty(rg_matrix);

cleanup:
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
	GrB_Index cap = Graph_RequiredMatrixDim(g);

	// This policy should only be used in a thread-safe context, so no locking is required.
	if(nrows != cap || ncols != cap) {
		GrB_Info res = GxB_Matrix_resize(m, cap, cap);
		ASSERT(res == GrB_SUCCESS);
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
			ASSERT(false);
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

	bool maintain_transpose;
	Config_Option_get(Config_MAINTAIN_TRANSPOSE, &maintain_transpose);

	if(maintain_transpose) {
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

	g->nodes                =  DataBlock_New(node_cap, sizeof(Entity), (fpDestructor)FreeEntity);
	g->edges                =  DataBlock_New(edge_cap, sizeof(Entity), (fpDestructor)FreeEntity);
	g->labels               =  array_new(RG_Matrix, GRAPH_DEFAULT_LABEL_CAP);
	g->relations            =  array_new(RG_Matrix, GRAPH_DEFAULT_RELATION_TYPE_CAP);
	g->adjacency_matrix     =  RG_Matrix_New(g, GrB_BOOL);
	g->_t_adjacency_matrix  =  RG_Matrix_New(g, GrB_BOOL);
	g->_zero_matrix         =  RG_Matrix_New(g, GrB_BOOL);

	// init graph statistics
	GraphStatistics_init(&g->stats);

	// If we're maintaining transposed relation matrices, allocate a new array, otherwise NULL-set the pointer.
	bool maintain_transpose;
	Config_Option_get(Config_MAINTAIN_TRANSPOSE, &maintain_transpose);
	g->t_relations = maintain_transpose ?
					 array_new(RG_Matrix, GRAPH_DEFAULT_RELATION_TYPE_CAP) : NULL;

	// Initialize a read-write lock scoped to the individual graph
	int res;
	UNUSED(res);
	res = pthread_rwlock_init(&g->_rwlock, NULL);
	ASSERT(res == 0);
	g->_writelocked = false;

	// Force GraphBLAS updates and resize matrices to node count by default
	Graph_SetMatrixPolicy(g, SYNC_AND_MINIMIZE_SPACE);

	// Synchronization objects initialization.
	res = pthread_mutex_init(&g->_writers_mutex, NULL);
	ASSERT(res == 0);

	// Create edge accumulator binary function
	if(!_graph_edge_accum) {
		GrB_Info info;
		UNUSED(info);
		info = GrB_BinaryOp_new(&_graph_edge_accum, _edge_accum, GrB_UINT64, GrB_UINT64, GrB_UINT64);
		ASSERT(info == GrB_SUCCESS);
	}

	return g;
}

// All graph matrices are required to be squared NXN
// where N = Graph_RequiredMatrixDim.
inline size_t Graph_RequiredMatrixDim(const Graph *g) {
	return _Graph_NodeCap(g);
}

size_t Graph_NodeCount(const Graph *g) {
	ASSERT(g);
	return g->nodes->itemCount;
}

uint Graph_DeletedNodeCount(const Graph *g) {
	ASSERT(g);
	return DataBlock_DeletedItemsCount(g->nodes);
}

size_t Graph_UncompactedNodeCount(const Graph *g) {
	return Graph_NodeCount(g) + Graph_DeletedNodeCount(g);
}

size_t Graph_LabeledNodeCount(const Graph *g, int label) {
	GrB_Index nvals = 0;
	GrB_Matrix m = Graph_GetLabelMatrix(g, label);
	if(m) GrB_Matrix_nvals(&nvals, m);
	return nvals;
}

size_t Graph_EdgeCount(const Graph *g) {
	ASSERT(g);
	return g->edges->itemCount;
}

uint64_t Graph_RelationEdgeCount(const Graph *g, int relation_idx) {
	return GraphStatistics_EdgeCount(&g->stats, relation_idx);
}

uint Graph_DeletedEdgeCount(const Graph *g) {
	ASSERT(g);
	return DataBlock_DeletedItemsCount(g->edges);
}

int Graph_RelationTypeCount(const Graph *g) {
	return array_len(g->relations);
}

int Graph_LabelTypeCount(const Graph *g) {
	return array_len(g->labels);
}

void Graph_AllocateNodes(Graph *g, size_t n) {
	ASSERT(g);
	DataBlock_Accommodate(g->nodes, n);
}

void Graph_AllocateEdges(Graph *g, size_t n) {
	ASSERT(g);
	DataBlock_Accommodate(g->edges, n);
}

int Graph_GetNode(const Graph *g, NodeID id, Node *n) {
	ASSERT(g);
	n->entity = _Graph_GetEntity(g->nodes, id);
	n->id = id;
	return (n->entity != NULL);
}

int Graph_GetEdge(const Graph *g, EdgeID id, Edge *e) {
	ASSERT(g && id < _Graph_EdgeCap(g));
	e->entity = _Graph_GetEntity(g->edges, id);
	e->id = id;
	return (e->entity != NULL);
}

int Graph_GetNodeLabel(const Graph *g, NodeID nodeID) {
	ASSERT(g);
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
	ASSERT(g && e);
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
	ASSERT(false);
	return GRAPH_NO_RELATION;
}

void Graph_GetEdgesConnectingNodes(const Graph *g, NodeID srcID, NodeID destID,
								   int r, Edge **edges) {
	ASSERT(g && r < Graph_RelationTypeCount(g) && edges);

	// Invalid relation type specified; this can occur on multi-type traversals like:
	// MATCH ()-[:real_type|fake_type]->()
	if(r == GRAPH_UNKNOWN_RELATION) return;

	Node srcNode = GE_NEW_NODE();
	Node destNode = GE_NEW_NODE();
	ASSERT(Graph_GetNode(g, srcID, &srcNode));
	ASSERT(Graph_GetNode(g, destID, &destNode));

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
	ASSERT(g);

	NodeID id;
	Entity *en = DataBlock_AllocateItem(g->nodes, &id);
	n->id = id;
	n->entity = en;
	n->labelID = label;
	en->prop_count = 0;
	en->properties = NULL;

	if(label != GRAPH_NO_LABEL) {
		// Try to set matrix at position [id, id]
		// incase of a failure, scale matrix.
		RG_Matrix matrix = g->labels[label];
		GrB_Matrix m = RG_Matrix_Get_GrB_Matrix(matrix);
		GrB_Info res = GrB_Matrix_setElement_BOOL(m, true, id, id);
		if(res != GrB_SUCCESS) {
			_MatrixResizeToCapacity(g, matrix);
			res = GrB_Matrix_setElement_BOOL(m, true, id, id);
			ASSERT(res == GrB_SUCCESS);
		}
		_Graph_SetLabelMatrixDirty(g, label);
	}
}

void Graph_FormConnection(Graph *g, NodeID src, NodeID dest, EdgeID edge_id, int r) {
	GrB_Info info;
	UNUSED(info);
	RG_Matrix M = g->relations[r];
	GrB_Matrix t_relationMat = NULL;
	GrB_Matrix adj = Graph_GetAdjacencyMatrix(g);
	GrB_Matrix tadj = Graph_GetTransposedAdjacencyMatrix(g);
	GrB_Matrix relationMat = Graph_GetRelationMatrix(g, r);

	bool maintain_transpose;
	Config_Option_get(Config_MAINTAIN_TRANSPOSE, &maintain_transpose);
	if(maintain_transpose) {
		t_relationMat = Graph_GetTransposedRelationMatrix(g, r);
	}

	// Rows represent source nodes, columns represent destination nodes.
	edge_id = SET_MSB(edge_id);
	GrB_Matrix_setElement_BOOL(adj, true, src, dest);
	GrB_Matrix_setElement_BOOL(tadj, true, dest, src);

	// set matrices as dirty
	_Graph_SetRelationMatrixDirty(g, r);
	_Graph_SetAdjacencyMatrixDirty(g);

	// An edge of type r has just been created, update statistics.
	GraphStatistics_IncEdgeCount(&g->stats, r, 1);

	// Matrix multi-edge is enable for this matrix, use GxB_Matrix_subassign.
	if(_RG_Matrix_MultiEdgeEnabled(M)) {
		GrB_Index I = src;
		GrB_Index J = dest;
		info = GxB_Matrix_subassign_UINT64   // C(I,J)<Mask> = accum (C(I,J),x)
			   (
				   relationMat,          // input/output matrix for results
				   GrB_NULL,             // optional mask for C(I,J), unused if NULL
				   _graph_edge_accum,    // optional accum for Z=accum(C(I,J),x)
				   edge_id,              // scalar to assign to C(I,J)
				   &I,                   // row indices
				   1,                    // number of row indices
				   &J,                   // column indices
				   1,                    // number of column indices
				   GrB_NULL              // descriptor for C(I,J) and Mask
			   );
		ASSERT(info == GrB_SUCCESS);

		// Update the transposed matrix if one is present.
		if(t_relationMat != NULL) {
			// Perform the same update to the J,I coordinates of the transposed matrix.
			info = GxB_Matrix_subassign_UINT64   // C(I,J)<Mask> = accum (C(I,J),x)
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
			ASSERT(info == GrB_SUCCESS);
		}
	} else {
		// Multi-edge is disabled, use GrB_Matrix_setElement.
		info = GrB_Matrix_setElement_UINT64(relationMat, edge_id, src, dest);
		ASSERT(info == GrB_SUCCESS);

		// Update the transposed matrix if one is present.
		if(t_relationMat != NULL) {
			info = GrB_Matrix_setElement_UINT64(t_relationMat, edge_id, dest, src);
			ASSERT(info == GrB_SUCCESS);
		}
	}
}

int Graph_ConnectNodes(Graph *g, NodeID src, NodeID dest, int r, Edge *e) {
	Node srcNode = GE_NEW_NODE();
	Node destNode = GE_NEW_NODE();

	int res;
	UNUSED(res);
	res = Graph_GetNode(g, src, &srcNode);
	ASSERT(res == 1);
	res = Graph_GetNode(g, dest, &destNode);
	ASSERT(res == 1);
	ASSERT(g && r < Graph_RelationTypeCount(g));

	EdgeID id;
	Entity *en = DataBlock_AllocateItem(g->edges, &id);
	en->prop_count = 0;
	en->properties = NULL;
	e->id = id;
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
	ASSERT(g && n && edges);
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
	ASSERT(g != NULL);
	ASSERT(e != NULL);

	uint64_t    x;
	GrB_Matrix  R;
	GrB_Matrix  M;
	GrB_Info    info;
	EdgeID      edge_id;
	GrB_Matrix  TR        =  GrB_NULL;
	int         r         =  Edge_GetRelationID(e);
	NodeID      src_id    =  Edge_GetSrcNodeID(e);
	NodeID      dest_id   =  Edge_GetDestNodeID(e);

	R = Graph_GetRelationMatrix(g, r);
	bool maintain_transpose;
	Config_Option_get(Config_MAINTAIN_TRANSPOSE, &maintain_transpose);
	if(maintain_transpose) TR = Graph_GetTransposedRelationMatrix(g, r);

	// test to see if edge exists
	info = GrB_Matrix_extractElement(&edge_id, R, src_id, dest_id);
	if(info != GrB_SUCCESS) return 0;

	// an edge of type r has just been deleted, update statistics
	GraphStatistics_DecEdgeCount(&g->stats, r, 1);

	if(SINGLE_EDGE(edge_id)) {
		// single edge of type R connecting src to dest, delete entry
		info = GxB_Matrix_Delete(R, src_id, dest_id);
		ASSERT(info == GrB_SUCCESS);
		if(TR) {
			info = GxB_Matrix_Delete(TR, dest_id, src_id);
			ASSERT(info == GrB_SUCCESS);
		}

		// see if source is connected to destination with additional edges
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

		// there are no additional edges connecting source to destination
		// remove edge from THE adjacency matrix
		if(!connected) {
			M = Graph_GetAdjacencyMatrix(g);
			info = GxB_Matrix_Delete(M, src_id, dest_id);
			ASSERT(info == GrB_SUCCESS);

			M = Graph_GetTransposedAdjacencyMatrix(g);
			info = GxB_Matrix_Delete(M, dest_id, src_id);
			ASSERT(info == GrB_SUCCESS);
		}
	} else {
		// multiple edges connecting src to dest
		// locate specific edge and remove it
		// revert back from array representation to edge ID
		// incase we're left with a single edge connecting src to dest

		int i = 0;
		EdgeID id = ENTITY_GET_ID(e);
		EdgeID *edges = (EdgeID *)edge_id;
		int edge_count = array_len(edges);

		// locate edge within edge array
		for(; i < edge_count; i++) if(edges[i] == id) break;
		ASSERT(i < edge_count);

		// remove edge from edge array
		// migrate last edge ID and reduce array size
		// TODO: reallocate array of size / capacity ratio is high
		array_del_fast(edges, i);

		// incase we're left with a single edge connecting src to dest
		// revert back from array to scalar
		if(array_len(edges) == 1) {
			edge_id = edges[0];
			array_free(edges);
			GrB_Matrix_setElement(R, SET_MSB(edge_id), src_id, dest_id);
		}

		if(TR) {
			// we must make the matching updates to the transposed matrix
			// first, extract the element that is known to be an edge array
			info = GrB_Matrix_extractElement(&edge_id, TR, dest_id, src_id);
			ASSERT(info == GrB_SUCCESS);
			edges = (EdgeID *)edge_id;
			// replace the deleted edge with the last edge in the matrix
			array_del_fast(edges, i);
			// free and replace the array if it now has 1 element
			if(array_len(edges) == 1) {
				edge_id = edges[0];
				array_free(edges);
				GrB_Matrix_setElement(TR, SET_MSB(edge_id), dest_id, src_id);
			}
		}
	}

	_Graph_SetAdjacencyMatrixDirty(g);
	_Graph_SetRelationMatrixDirty(g, r);

	// free and remove edges from datablock.
	DataBlock_DeleteItem(g->edges, ENTITY_GET_ID(e));
	return 1;
}

inline bool Graph_EntityIsDeleted(Entity *e) {
	return DataBlock_ItemIsDeleted(e);
}

void Graph_DeleteNode(Graph *g, Node *n) {
	/* Assumption, node is completely detected,
	 * there are no incoming nor outgoing edges
	 * leading to / from node. */
	ASSERT(g && n);

	// Clear label matrix at position node ID.
	uint32_t label_count = array_len(g->labels);
	for(int i = 0; i < label_count; i++) {
		GrB_Matrix M = Graph_GetLabelMatrix(g, i);
		GxB_Matrix_Delete(M, ENTITY_GET_ID(n), ENTITY_GET_ID(n));
	}

	DataBlock_DeleteItem(g->nodes, ENTITY_GET_ID(n));
}

static void _Graph_FreeRelationMatrices(Graph *g) {
	// TODO: disable datablock deleted items array
	// there's no need to keep track after deleted items as the graph
	// is being removed we won't be reusing items

	if(!_binary_op_delete_edges) {
		// The binary operator has not yet been constructed; build it now.
		GrB_Info res;
		UNUSED(res);
		res = GrB_BinaryOp_new(&_binary_op_delete_edges, _binary_op_free_edge,
							   GrB_UINT64, GrB_UINT64, GrB_UINT64);
		ASSERT(res == GrB_SUCCESS);
	}

	GxB_Scalar thunk;
	GxB_Scalar_new(&thunk, GrB_UINT64);
	GxB_Scalar_setElement_UINT64(thunk, (uint64_t)g);

	bool maintain_transpose;
	Config_Option_get(Config_MAINTAIN_TRANSPOSE, &maintain_transpose);
	uint relationCount = Graph_RelationTypeCount(g);
	/* use the edge deletion binary operation to free all edge arrays within
	 * the adjacency matrix */

	for(uint i = 0; i < relationCount; i++) {
		RG_Matrix  M = g->relations[i];
		GrB_Matrix C = M->grb_matrix;

		GxB_Matrix_apply_BinaryOp1st(C, GrB_NULL, GrB_NULL,
									 _binary_op_delete_edges, thunk, C, GrB_NULL);

		// free the matrix itself
		RG_Matrix_Free(M);

		// perform the same update to transposed matrices
		if(maintain_transpose) {
			RG_Matrix TM = g->t_relations[i];
			C = TM->grb_matrix;

			GxB_Matrix_apply_BinaryOp1st(C, GrB_NULL, GrB_NULL,
										 _binary_op_delete_edges, thunk, C, GrB_NULL);

			// free the matrix itself
			RG_Matrix_Free(TM);
		}
	}
	GrB_free(&thunk);
}

static void _BulkDeleteImplicitEdges(Graph *g, GrB_Matrix Mask) {
	GrB_Matrix          adj;        // adjacency matrix
	GrB_Matrix          tadj;       // transposed adjacency matrix
	GrB_Index           nrows;
	GrB_Index           ncols;
	GrB_Matrix          A;          // a = R(M) masked relation matrix
	GrB_Descriptor      desc;       // GraphBLAS descriptor

	nrows = Graph_RequiredMatrixDim(g);
	ncols = nrows;
	GrB_Descriptor_new(&desc);
	adj = Graph_GetAdjacencyMatrix(g);
	tadj = Graph_GetTransposedAdjacencyMatrix(g);
	GrB_Matrix_new(&A, GrB_UINT64, nrows, ncols);

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

	// clear updated output matrix before assignment
	GrB_Descriptor_set(desc, GrB_OUTP, GrB_REPLACE);

	// free and remove implicit edges from relation matrices
	int relation_count = Graph_RelationTypeCount(g);
	for(int i = 0; i < relation_count; i++) {
		GrB_Matrix R = Graph_GetRelationMatrix(g, i);

		// reset mask descriptor
		GrB_Descriptor_set(desc, GrB_MASK, GxB_DEFAULT);

		/* isolate implicit edges
		 * A will contain all implicitly deleted edges from R */
		GrB_Matrix_apply(A, Mask, GrB_NULL, GrB_IDENTITY_UINT64, R, desc);

		uint64_t edges_before_deletion = Graph_EdgeCount(g);
		// The number of deleted edges is equals the diff in the number of items in the DataBlock
		uint64_t n_deleted_edges = edges_before_deletion - Graph_EdgeCount(g);

		// free each multi edge array entry in A
		GxB_Matrix_apply_BinaryOp1st(A, GrB_NULL, GrB_NULL,
									 _binary_op_delete_edges, thunk, A, GrB_NULL);

		// Multiple edges of type r has just been deleted, update statistics
		GraphStatistics_DecEdgeCount(&g->stats, i, n_deleted_edges);
		// clear the relation matrix
		GrB_Descriptor_set(desc, GrB_MASK, GrB_COMP);
		GrB_Descriptor_set(desc, GrB_MASK, GrB_STRUCTURE);

		// remove every entry of R marked by Mask
		GrB_Matrix_apply(R, Mask, GrB_NULL, GrB_IDENTITY_UINT64, R, desc);
		_Graph_SetRelationMatrixDirty(g, i);
	}

	// reset mask descriptor
	GrB_Descriptor_set(desc, GrB_MASK, GrB_COMP);
	GrB_Descriptor_set(desc, GrB_MASK, GrB_STRUCTURE);

	// update the adjacency matrix to remove deleted entries
	GrB_Matrix_apply(adj, Mask, GrB_NULL, GrB_IDENTITY_BOOL, adj, desc);

	// transpose the mask so that it will match the transposed adjacency matrix
	GrB_transpose(Mask, GrB_NULL, GrB_NULL, Mask, GrB_NULL);

	// update the transposed adjacency matrix
	GrB_Matrix_apply(tadj, Mask, GrB_NULL, GrB_IDENTITY_BOOL, tadj, desc);

	/* if we have individual transposed matrices, repeat all the above steps
	 * with the transposed Mask */
	bool maintain_transpose;
	Config_Option_get(Config_MAINTAIN_TRANSPOSE, &maintain_transpose);
	if(maintain_transpose) {
		for(int i = 0; i < relation_count; i++) {
			GrB_Matrix TR = Graph_GetTransposedRelationMatrix(g, i);

			// reset mask descriptor
			GrB_Descriptor_set(desc, GrB_MASK, GxB_DEFAULT);

			/* isolate implicit edges
			 * A will contain all implicitly deleted edges from TR */
			GrB_Matrix_apply(A, Mask, GrB_NULL, GrB_IDENTITY_UINT64, TR, desc);

			// free each multi edge array entry in A
			GxB_Matrix_apply_BinaryOp1st(A, GrB_NULL, GrB_NULL,
										 _binary_op_delete_edges, thunk, A, GrB_NULL);

			// clear the relation matrix
			GrB_Descriptor_set(desc, GrB_MASK, GrB_COMP);
			GrB_Descriptor_set(desc, GrB_MASK, GrB_STRUCTURE);

			// remove every entry of TR marked by Mask
			GrB_Matrix_apply(TR, Mask, GrB_NULL, GrB_IDENTITY_UINT64, TR, desc);
		}
	}
	// Clean up.
	GrB_free(&A);
	GrB_free(&thunk);
	GrB_Descriptor_free(&desc);
}

static void _BulkDeleteNodes(Graph *g, Node *nodes, uint node_count,
							 uint *node_deleted, uint *edge_deleted) {
	ASSERT(g && g->_writelocked && nodes && node_count > 0);

	if(!_binary_op_delete_edges) {
		// The binary operator has not yet been constructed; build it now.
		GrB_Info res;
		UNUSED(res);
		res = GrB_BinaryOp_new(&_binary_op_delete_edges, _binary_op_free_edge,
							   GrB_UINT64, GrB_UINT64, GrB_UINT64);
		ASSERT(res == GrB_SUCCESS);
	}

	/* Create a matrix M where M[j,i] = 1 if:
	 * Node i is connected to node j. */

	GrB_Matrix          adj;        // adjacency matrix
	GrB_Matrix          tadj;       // transposed adjacency matrix
	GrB_Index           nrows;
	GrB_Index           ncols;
	GrB_Index           nvals;      // number of elements in mask
	GrB_Matrix          Mask;       // mask noteing all implicitly deleted edges
	GrB_Matrix          Nodes;      // mask noteing each node marked for deletion
	GrB_Descriptor      desc;       // GraphBLAS descriptor
	GxB_MatrixTupleIter *adj_iter;  // iterator over the adjacency matrix
	GxB_MatrixTupleIter *tadj_iter; // iterator over the transposed adjacency matrix

	adj                         =  Graph_GetAdjacencyMatrix(g);
	tadj                        =  Graph_GetTransposedAdjacencyMatrix(g);
	nrows                       =  Graph_RequiredMatrixDim(g);
	ncols                       =  nrows;
	GrB_Descriptor_new(&desc);
	GxB_MatrixTupleIter_new(&adj_iter, adj);
	GxB_MatrixTupleIter_new(&tadj_iter, tadj);

	// implicit deleted edge, set format to hypersparse
	// expecting a small number of implicit deleted edges
	GrB_Matrix_new(&Mask, GrB_BOOL, nrows, ncols);
	GxB_Matrix_Option_set(Mask, GxB_SPARSITY_CONTROL, GxB_HYPERSPARSE);

	GrB_Matrix_new(&Nodes, GrB_BOOL, nrows, ncols);

	// mark matrices as dirty
	_Graph_SetAdjacencyMatrixDirty(g);

	// populate mask with implicit edges, take note of deleted nodes
	for(uint i = 0; i < node_count; i++) {
		GrB_Index src;
		GrB_Index dest;
		Node *n = nodes + i;
		bool depleted = false;
		NodeID ID = ENTITY_GET_ID(n);

		// outgoing edges
		GxB_MatrixTupleIter_iterate_row(adj_iter, ID);
		while(true) {
			GxB_MatrixTupleIter_next(adj_iter, NULL, &dest, &depleted);
			if(depleted) break;
			GrB_Matrix_setElement_BOOL(Mask, true, ID, dest);
		}

		depleted = false;

		// incoming edges
		GxB_MatrixTupleIter_iterate_row(tadj_iter, ID);
		while(true) {
			GxB_MatrixTupleIter_next(tadj_iter, NULL, &src, &depleted);
			if(depleted) break;
			GrB_Matrix_setElement_BOOL(Mask, true, src, ID);
		}

		// node to remove
		GrB_Matrix_setElement_BOOL(Nodes, true, ID, ID);
	}

	// update deleted node count
	GrB_Matrix_nvals(&nvals, Nodes);
	*node_deleted += nvals;

	// update deleted edge count
	GrB_Matrix_nvals(&nvals, Mask);
	*edge_deleted += nvals;

	if(nvals <= EDGE_BULK_DELETE_THRESHOLD) {
		// small number of implicit edges to delete
		GrB_Index  src_id;
		GrB_Index  dest_id;
		GrB_Type   type;
		GrB_Index  *Ap;
		GrB_Index  *Ah;
		GrB_Index  *Aj;
		void       *Ax;
		GrB_Index  Ap_size;
		GrB_Index  Ah_size;
		GrB_Index  Aj_size;
		GrB_Index  Ax_size;
		GrB_Index  nvec;
		bool jumbled = true;
		Edge *edges = array_new(Edge, 1);

		// export and free a hypersparse CSR matrix
		GxB_Matrix_export_HyperCSR(
			&Mask,        // handle of matrix to export and free
			&type,        // type of matrix exported
			&nrows,       // number of rows of the matrix
			&ncols,       // number of columns of the matrix
			&Ap,          // row "pointers", Ap_size >= nvec+1
			&Ah,          // row indices, Ah_size >= nvec
			&Aj,          // column indices, Aj_size >= nvals(A)
			&Ax,          // values, Ax_size >= nvals(A)
			&Ap_size,     // size of Ap
			&Ah_size,     // size of Ah
			&Aj_size,     // size of Aj
			&Ax_size,     // size of Ax
			&nvec,        // number of rows that appear in Ah
			&jumbled,     // if true, indices in each row may be unsorted
			GrB_NULL
		);

		for(GrB_Index k = 0 ; k < nvec ; k++) {
			src_id = Ah [k] ;
			for(GrB_Index p = Ap [k] ; p < Ap [k + 1] ; p++) {
				dest_id = Aj[p];
				// retrieve all edges connecting this source and destination
				Graph_GetEdgesConnectingNodes(g, src_id, dest_id,
											  GRAPH_NO_RELATION, &edges);
				uint edge_count = array_len(edges);
				for(uint i = 0; i < edge_count; i ++) {
					Graph_DeleteEdge(g, &edges[i]);
				}
				array_clear(edges);
			}
		}
		// clean up
		rm_free(Ap);
		rm_free(Ah);
		rm_free(Aj);
		rm_free(Ax);
		array_free(edges);
	} else {
		// use bulk deletion logic to remove implicit edges
		_BulkDeleteImplicitEdges(g, Mask);
	}

	/* delete nodes
	 * all nodes marked for deletion are detected, no incoming / outgoing edges. */
	GrB_Descriptor_set(desc, GrB_OUTP, GrB_REPLACE);
	GrB_Descriptor_set(desc, GrB_MASK, GrB_COMP);
	GrB_Descriptor_set(desc, GrB_MASK, GrB_STRUCTURE);
	int label_count = Graph_LabelTypeCount(g);

	// track all labels that contain deleted nodes
	bool deleted_labels[label_count];
	memset(deleted_labels, 0, label_count * sizeof(bool));

	// TODO: use the apply operator to delete datablock entries
	for(uint i = 0; i < node_count; i++) {
		Node *n = nodes + i;
		// mark label as containing deletions
		int label_id = NODE_GET_LABEL_ID(n, g);
		if(label_id != GRAPH_NO_LABEL) deleted_labels[label_id] = true;

		DataBlock_DeleteItem(g->nodes, ENTITY_GET_ID(n));
	}

	// clear deleted nodes from label matrices
	for(int i = 0; i < label_count; i++) {
		if(deleted_labels[i] == 0) continue; // label did not change
		GrB_Matrix L = Graph_GetLabelMatrix(g, i);
		GrB_Matrix_apply(L, Nodes, GrB_NULL, GrB_IDENTITY_BOOL, L, desc);
		_Graph_SetLabelMatrixDirty(g, i);
	}

	// Clean up.
	GrB_free(&desc);
	if(Mask) GrB_free(&Mask);
	GrB_free(&Nodes);
	GxB_MatrixTupleIter_free(adj_iter);
	GxB_MatrixTupleIter_free(tadj_iter);
}

static void _BulkDeleteEdges(Graph *g, Edge *edges, size_t edge_count) {
	ASSERT(g && g->_writelocked && edges && edge_count > 0);

	int relationCount = Graph_RelationTypeCount(g);
	GrB_Matrix masks[relationCount];
	for(int i = 0; i < relationCount; i++) masks[i] = NULL;
	bool update_adj_matrices = false;

	bool maintain_transpose;
	Config_Option_get(Config_MAINTAIN_TRANSPOSE, &maintain_transpose);

	GrB_Index n = Graph_RequiredMatrixDim(g);

	for(int i = 0; i < edge_count; i++) {
		Edge *e = edges + i;
		int r = Edge_GetRelationID(e);
		NodeID src_id = Edge_GetSrcNodeID(e);
		NodeID dest_id = Edge_GetDestNodeID(e);
		EdgeID edge_id;
		GrB_Matrix R = Graph_GetRelationMatrix(g, r);  // Relation matrix.
		GrB_Matrix TR = maintain_transpose ? Graph_GetTransposedRelationMatrix(g, r) : NULL;
		GrB_Matrix_extractElement(&edge_id, R, src_id, dest_id);

		// An edge of type r has just been deleted, update statistics.
		GraphStatistics_DecEdgeCount(&g->stats, r, 1);

		if(SINGLE_EDGE(edge_id)) {
			update_adj_matrices = true;
			GrB_Matrix mask = masks[r];    // mask noteing all deleted edges.
			// Get mask of this relation type.
			if(mask == NULL) {
				GrB_Matrix_new(&mask, GrB_BOOL, n, n);
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
			ASSERT(i < multi_edge_count);

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
		GrB_Matrix_new(&remaining_mask, GrB_BOOL, n, n);
		GrB_Descriptor desc;    // GraphBLAS descriptor.
		GrB_Descriptor_new(&desc);
		// Descriptor sets to clear entry according to mask.
		GrB_Descriptor_set(desc, GrB_MASK, GrB_COMP);

		// Clear updated output matrix before assignment.
		GrB_Descriptor_set(desc, GrB_OUTP, GrB_REPLACE);

		bool maintain_transpose;
		Config_Option_get(Config_MAINTAIN_TRANSPOSE, &maintain_transpose);
		for(int r = 0; r < relationCount; r++) {
			GrB_Matrix mask = masks[r];
			GrB_Matrix R = Graph_GetRelationMatrix(g, r);  // Relation matrix.
			if(mask) {
				_Graph_SetRelationMatrixDirty(g, r);
				// Remove every entry of R marked by Mask.
				// Desc: GrB_MASK = GrB_COMP,  GrB_OUTP = GrB_REPLACE.
				// R = R & !mask.
				GrB_Matrix_apply(R, mask, GrB_NULL, GrB_IDENTITY_UINT64, R, desc);
				if(maintain_transpose) {
					GrB_Matrix tM = Graph_GetTransposedRelationMatrix(g, r);  // Transposed relation mapping matrix.
					// Transpose mask (this cannot be done by descriptor).
					GrB_transpose(mask, GrB_NULL, GrB_NULL, mask, GrB_NULL);
					// tM = tM & !mask.
					GrB_Matrix_apply(tM, mask, GrB_NULL, GrB_IDENTITY_UINT64, tM, desc);
				}
				GrB_free(&mask);
			}

			// Collect remaining edges. remaining_mask = remaining_mask + R.
			GrB_eWiseAdd(remaining_mask, GrB_NULL, GrB_NULL, GxB_ANY_PAIR_BOOL, remaining_mask,
						 R, GrB_NULL);
		}

		GrB_Matrix adj_matrix = Graph_GetAdjacencyMatrix(g);
		GrB_Matrix t_adj_matrix = Graph_GetTransposedAdjacencyMatrix(g);
		_Graph_SetAdjacencyMatrixDirty(g);

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
	ASSERT(g);

	uint _edge_deleted = 0;
	uint _node_deleted = 0;

	if(node_count) {
		_BulkDeleteNodes(g, nodes, node_count, &_node_deleted, &_edge_deleted);
	}

	if(edge_count) {
		// Filter out explicit edges which were removed by _BulkDeleteNodes.
		if(node_count) {
			for(int i = 0; i < edge_count; i++) {
				Edge *e = edges + i;
				NodeID src = Edge_GetSrcNodeID(e);
				NodeID dest = Edge_GetDestNodeID(e);

				if(!DataBlock_GetItem(g->nodes, src) || !DataBlock_GetItem(g->nodes, dest)) {
					// edge already removed due to node removal
					// replace current edge with last edge
					edges[i] = edges[edge_count - 1];

					// Update indices.
					i--;
					edge_count--;
				}
			}
		}

		// removing duplicates
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
		if(edge_count > 0) _BulkDeleteEdges(g, edges, edge_count);
	}
	_edge_deleted += edge_count;

	if(node_deleted != NULL) *node_deleted = _node_deleted;
	if(edge_deleted != NULL) *edge_deleted = _edge_deleted;
}

DataBlockIterator *Graph_ScanNodes(const Graph *g) {
	ASSERT(g);
	return DataBlock_Scan(g->nodes);
}

DataBlockIterator *Graph_ScanEdges(const Graph *g) {
	ASSERT(g);
	return DataBlock_Scan(g->edges);
}

int Graph_AddLabel(Graph *g) {
	ASSERT(g != NULL);

	GrB_Info info;
	RG_Matrix m = RG_Matrix_New(g, GrB_BOOL);

	/* matrix iterator requires matrix format to be sparse
	 * to avoid future conversion from HYPER-SPARSE, BITMAP, FULL to SPARSE
	 * we set matrix format at creation time, as Label matrices are iterated
	 * within the LabelScan Execution-Plan operation. */

	GrB_Matrix M = RG_Matrix_Get_GrB_Matrix(m);
	info = GxB_set(M, GxB_SPARSITY_CONTROL, GxB_SPARSE);
	UNUSED(info);
	ASSERT(info == GrB_SUCCESS);

	array_append(g->labels, m);
	return array_len(g->labels) - 1;
}

int Graph_AddRelationType(Graph *g) {
	ASSERT(g);

	RG_Matrix m = RG_Matrix_New(g, GrB_UINT64);
	array_append(g->relations, m);
	// Adding a new relationship type, update the stats structures to support it.
	GraphStatistics_IntroduceRelationship(&g->stats);
	bool maintain_transpose;
	Config_Option_get(Config_MAINTAIN_TRANSPOSE, &maintain_transpose);

	if(maintain_transpose) {
		RG_Matrix tm = RG_Matrix_New(g, GrB_UINT64);
		array_append(g->t_relations, tm);
	}

	int relationID = Graph_RelationTypeCount(g) - 1;
	return relationID;
}

GrB_Matrix Graph_GetAdjacencyMatrix(const Graph *g) {
	ASSERT(g);
	RG_Matrix m = g->adjacency_matrix;
	g->SynchronizeMatrix(g, m);
	return RG_Matrix_Get_GrB_Matrix(m);
}

// Get the transposed adjacency matrix.
GrB_Matrix Graph_GetTransposedAdjacencyMatrix(const Graph *g) {
	ASSERT(g);
	RG_Matrix m = g->_t_adjacency_matrix;
	g->SynchronizeMatrix(g, m);
	return RG_Matrix_Get_GrB_Matrix(m);
}

GrB_Matrix Graph_GetLabelMatrix(const Graph *g, int label_idx) {
	ASSERT(g && label_idx < array_len(g->labels));
	RG_Matrix m = g->labels[label_idx];
	g->SynchronizeMatrix(g, m);
	return RG_Matrix_Get_GrB_Matrix(m);
}

GrB_Matrix Graph_GetRelationMatrix(const Graph *g, int relation_idx) {
	ASSERT(g && (relation_idx == GRAPH_NO_RELATION || relation_idx < Graph_RelationTypeCount(g)));

	if(relation_idx == GRAPH_NO_RELATION) {
		return Graph_GetAdjacencyMatrix(g);
	} else {
		RG_Matrix m = g->relations[relation_idx];
		g->SynchronizeMatrix(g, m);
		return RG_Matrix_Get_GrB_Matrix(m);
	}
}

// Returns true if relationship matrix 'r' contains multi-edge entries, false otherwise
bool Graph_RelationshipContainsMultiEdge(const Graph *g, int r) {
	ASSERT(Graph_RelationTypeCount(g) > r);
	GrB_Index nvals;
	// A relationship matrix contains multi-edge if nvals < number of edges with type r.
	GrB_Matrix R = Graph_GetRelationMatrix(g, r);
	GrB_Matrix_nvals(&nvals, R);

	return (Graph_RelationEdgeCount(g, r) > nvals);
}

GrB_Matrix Graph_GetTransposedRelationMatrix(const Graph *g, int relation_idx) {
	ASSERT(g && (relation_idx == GRAPH_NO_RELATION || relation_idx < Graph_RelationTypeCount(g)));

	if(relation_idx == GRAPH_NO_RELATION) {
		return Graph_GetTransposedAdjacencyMatrix(g);
	} else {
		ASSERT(g->t_relations && "tried to retrieve nonexistent transposed matrix.");

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
	ASSERT(g);
	// Free matrices.
	Entity *en;
	DataBlockIterator *it;
	RG_Matrix_Free(g->_zero_matrix);
	RG_Matrix_Free(g->adjacency_matrix);
	RG_Matrix_Free(g->_t_adjacency_matrix);

	_Graph_FreeRelationMatrices(g);
	array_free(g->relations);
	array_free(g->t_relations);
	GraphStatistics_FreeInternals(&g->stats);

	uint32_t labelCount = array_len(g->labels);
	for(int i = 0; i < labelCount; i++) {
		RG_Matrix_Free(g->labels[i]);
	}
	array_free(g->labels);

	it = Graph_ScanNodes(g);
	while((en = (Entity *)DataBlockIterator_Next(it, NULL)) != NULL)
		FreeEntity(en);

	DataBlockIterator_Free(it);

	it = Graph_ScanEdges(g);
	while((en = DataBlockIterator_Next(it, NULL)) != NULL)
		FreeEntity(en);

	DataBlockIterator_Free(it);

	// Free blocks.
	DataBlock_Free(g->nodes);
	DataBlock_Free(g->edges);

	int res;
	UNUSED(res);
	res = pthread_mutex_destroy(&g->_writers_mutex);
	ASSERT(res == 0);

	if(g->_writelocked) Graph_ReleaseLock(g);
	res = pthread_rwlock_destroy(&g->_rwlock);
	ASSERT(res == 0);

	rm_free(g);
}

