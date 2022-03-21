/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "graph.h"
#include "../util/arr.h"
#include "../util/qsort.h"
#include "../util/rmalloc.h"
#include "../util/datablock/oo_datablock.h"
#include "../graph/rg_matrix/rg_matrix_iter.h"

//------------------------------------------------------------------------------
// Forward declarations
//------------------------------------------------------------------------------
void _MatrixResizeToCapacity(const Graph *g, RG_Matrix m);

//------------------------------------------------------------------------------
// Synchronization functions
//------------------------------------------------------------------------------

static void _CreateRWLock
(
	Graph *g
) {
	// create a read write lock which favors writes
	//
	// consider the following locking sequence:
	// T0 read lock  (acquired)
	// T1 write lock (waiting)
	// T2 read lock  (acquired if lock favor reads, waiting if favor writes)
	//
	// we don't want to cause write starvation as this can impact overall
	// system performance

	// specify prefer write in lock creation attributes
	int res = 0 ;
	UNUSED(res) ;

	pthread_rwlockattr_t attr ;
	res = pthread_rwlockattr_init(&attr) ;
	ASSERT(res == 0) ;

#if !defined(__APPLE__) && !defined(__FreeBSD__)
	int pref = PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP ;
	res = pthread_rwlockattr_setkind_np(&attr, pref) ;
	ASSERT(res == 0) ;
#endif

	res = pthread_rwlock_init(&g->_rwlock, &attr);
	ASSERT(res == 0) ;
}

// acquire a lock that does not restrict access from additional reader threads
void Graph_AcquireReadLock(Graph *g) {
	pthread_rwlock_rdlock(&g->_rwlock);
}

// acquire a lock for exclusive access to this graph's data
void Graph_AcquireWriteLock(Graph *g) {
	pthread_rwlock_wrlock(&g->_rwlock);
	g->_writelocked = true;
}

// Release the held lock
void Graph_ReleaseLock
(
	Graph *g
) {
	// set _writelocked to false BEFORE unlocking
	// if this is a reader thread no harm done,
	// if this is a writer thread the writer is about to unlock so once again
	// no harm done, if we set `_writelocked` to false after unlocking it is possible
	// for a reader thread to be considered as writer, performing illegal access to
	// underline matrices, consider a context switch after unlocking `_rwlock` but
	// before setting `_writelocked` to false
	g->_writelocked = false;
	pthread_rwlock_unlock(&g->_rwlock);
}

//------------------------------------------------------------------------------
// Graph utility functions
//------------------------------------------------------------------------------

// return number of nodes graph can contain
static inline size_t _Graph_NodeCap(const Graph *g) {
	return g->nodes->itemCap;
}

static void _CollectEdgesFromEntry
(
	const Graph *g,
	NodeID src,
	NodeID dest,
	int r,
	EdgeID edgeId,
	Edge **edges
) {
	Edge e = {0};

	e.relationID  =  r;
	e.srcNodeID   =  src;
	e.destNodeID  =  dest;

	if(SINGLE_EDGE(edgeId)) {
		e.id      =  edgeId;
		e.entity  =  DataBlock_GetItem(g->edges, edgeId);
		ASSERT(e.entity);
		array_append(*edges, e);
	} else {
		// multiple edges connecting src to dest,
		// entry is a pointer to an array of edge IDs
		EdgeID *edgeIds = (EdgeID *)(CLEAR_MSB(edgeId));
		uint edgeCount = array_len(edgeIds);

		for(uint i = 0; i < edgeCount; i++) {
			edgeId = edgeIds[i];
			e.entity = DataBlock_GetItem(g->edges, edgeId);
			e.id = edgeId;
			ASSERT(e.entity);
			array_append(*edges, e);
		}
	}
}

// Locates edges connecting src to destination.
void _Graph_GetEdgesConnectingNodes
(
	const Graph *g,
	NodeID src,
	NodeID dest,
	int r,
	Edge **edges
) {
	ASSERT(g);
	ASSERT(r    != GRAPH_NO_RELATION);
	ASSERT(r    < Graph_RelationTypeCount(g));
	ASSERT(src  < Graph_RequiredMatrixDim(g));
	ASSERT(dest < Graph_RequiredMatrixDim(g));

	// relation map, maps (src, dest, r) to edge IDs.
	EdgeID      id    =  INVALID_ENTITY_ID;
	RG_Matrix   M     =  Graph_GetRelationMatrix(g, r, false);
	GrB_Info    res   =  RG_Matrix_extractElement_UINT64(&id, M, src, dest);

	// no entry at [dest, src], src is not connected to dest with relation R
	if(res == GrB_NO_VALUE) return;

	_CollectEdgesFromEntry(g, src, dest, r, id, edges);
}

static inline Entity *_Graph_GetEntity(const DataBlock *entities, EntityID id) {
	return DataBlock_GetItem(entities, id);
}

//------------------------------------------------------------------------------
// Matrix synchronization and resizing functions
//------------------------------------------------------------------------------

// resize given matrix, such that its number of row and columns
// matches the number of nodes in the graph. Also, synchronize
// matrix to execute any pending operations
void _MatrixSynchronize
(
	const Graph *g,
	RG_Matrix m
) {
	bool dirty = RG_Matrix_isDirty(m);
	GrB_Info info;
	GrB_Index n_rows;
	GrB_Index n_cols;
	RG_Matrix_nrows(&n_rows, m);
	RG_Matrix_ncols(&n_cols, m);
	GrB_Index dims = Graph_RequiredMatrixDim(g);

	UNUSED(info);

	// matrix must be resized if its dimensions missmatch required dimensions
	bool require_resize = (n_rows != dims || n_cols != dims);

	// matrix fully synced, nothing to do
	if(!require_resize && !RG_Matrix_isDirty(m)) return;

	// lock the matrix
	RG_Matrix_Lock(m);

	// recheck
	RG_Matrix_nrows(&n_rows, m);
	RG_Matrix_ncols(&n_cols, m);
	dims = Graph_RequiredMatrixDim(g);
	require_resize = (n_rows != dims || n_cols != dims);

	// some other thread performed sync
	if(!require_resize && !RG_Matrix_isDirty(m)) goto cleanup;

	// resize if required
	if(require_resize) {
		info = RG_Matrix_resize(m, dims, dims);
		ASSERT(info == GrB_SUCCESS);
	}

	// flush pending changes if dirty
	if(RG_Matrix_isDirty(m)) {
		info = RG_Matrix_wait(m, false);
		ASSERT(info == GrB_SUCCESS);
	}

cleanup:
	// Unlock matrix mutex.
	RG_Matrix_Unlock(m);
}

// resize matrix to node capacity
void _MatrixResizeToCapacity
(
	const Graph *g,
	RG_Matrix m
) {
	GrB_Index nrows;
	GrB_Index ncols;
	RG_Matrix_ncols(&ncols, m);
	RG_Matrix_nrows(&nrows, m);
	GrB_Index cap = Graph_RequiredMatrixDim(g);

	// this policy should only be used in a thread-safe context,
	// so no locking is required
	if(nrows != cap || ncols != cap) {
		GrB_Info res = RG_Matrix_resize(m, cap, cap);
		ASSERT(res == GrB_SUCCESS);
	}
}

// do not update matrices
void _MatrixNOP
(
	const Graph *g,
	RG_Matrix matrix
) {
	return;
}

MATRIX_POLICY Graph_GetMatrixPolicy
(
	const Graph *g
) {
	ASSERT(g != NULL);
	MATRIX_POLICY policy = SYNC_POLICY_UNKNOWN;
	SyncMatrixFunc f = g->SynchronizeMatrix;

	if(f == _MatrixSynchronize) {
		policy = SYNC_POLICY_FLUSH_RESIZE;
	} else if(f == _MatrixResizeToCapacity) {
		policy = SYNC_POLICY_RESIZE;
	} else if(f == _MatrixNOP) {
		policy = SYNC_POLICY_NOP;
	} else {
		ASSERT(false);
	}

	return policy;
}

// define the current behavior for matrix creations and retrievals on this graph
void Graph_SetMatrixPolicy
(
	Graph *g,
	MATRIX_POLICY policy
) {
	switch(policy) {
		case SYNC_POLICY_FLUSH_RESIZE:
			// Default behavior; forces execution of pending GraphBLAS operations
			// when appropriate and sizes matrices to the current node count.
			g->SynchronizeMatrix = _MatrixSynchronize;
			break;
		case SYNC_POLICY_RESIZE:
			// Bulk insertion and creation behavior; does not force pending operations
			// and resizes matrices to the graph's current node capacity.
			g->SynchronizeMatrix = _MatrixResizeToCapacity;
			break;
		case SYNC_POLICY_NOP:
			// Used when deleting or freeing a graph; forces no matrix updates or resizes.
			g->SynchronizeMatrix = _MatrixNOP;
			break;
		default:
			ASSERT(false);
	}
}

// synchronize and resize all matrices in graph
void Graph_ApplyAllPending
(
	Graph *g,
	bool force_flush
) {
	ASSERT(g != NULL);

	uint       n  =  0;
	RG_Matrix  M  =  NULL;

	M = Graph_GetAdjacencyMatrix(g, false);
	RG_Matrix_wait(M, force_flush);

	M = Graph_GetNodeLabelMatrix(g);
	RG_Matrix_wait(M, force_flush);

	n = array_len(g->labels);
	for(int i = 0; i < n; i ++) {
		M = Graph_GetLabelMatrix(g, i);
		RG_Matrix_wait(M, force_flush);
	}

	n = array_len(g->relations);
	for(int i = 0; i < n; i ++) {
		M = Graph_GetRelationMatrix(g, i, false);
		RG_Matrix_wait(M, force_flush);
	}
}

bool Graph_Pending
(
	const Graph *g
) {
	ASSERT(g != NULL);

	GrB_Info   info;
	UNUSED(info);

	uint       n        =  0;
	RG_Matrix  M        =  NULL;
	bool       pending  =  false;

	//--------------------------------------------------------------------------
	// see if ADJ matrix contains pending changes
	//--------------------------------------------------------------------------

	M = g->adjacency_matrix;
	info = RG_Matrix_pending(M, &pending);
	ASSERT(info == GrB_SUCCESS);
	if(pending) return true;

	//--------------------------------------------------------------------------
	// see if any label matrix contains pending changes
	//--------------------------------------------------------------------------

	n = array_len(g->labels);
	for(int i = 0; i < n; i ++) {
		M = g->labels[i];
		info = RG_Matrix_pending(M, &pending);
		ASSERT(info == GrB_SUCCESS);
		if(pending) return true;
	}

	//--------------------------------------------------------------------------
	// see if any relationship matrix contains pending changes
	//--------------------------------------------------------------------------

	n = array_len(g->relations);
	for(int i = 0; i < n; i ++) {
		M = g->relations[i];
		info = RG_Matrix_pending(M, &pending);
		ASSERT(info == GrB_SUCCESS);
		if(pending) return true;
	}

	return false;
}

//------------------------------------------------------------------------------
// Graph API
//------------------------------------------------------------------------------

Graph *Graph_New
(
	size_t node_cap,
	size_t edge_cap
) {

	fpDestructor cb = (fpDestructor)FreeEntity;
	Graph *g = rm_calloc(1, sizeof(Graph));

	g->nodes      =  DataBlock_New(node_cap, node_cap, sizeof(Entity), cb);
	g->edges      =  DataBlock_New(edge_cap, edge_cap, sizeof(Entity), cb);
	g->labels     =  array_new(RG_Matrix, GRAPH_DEFAULT_LABEL_CAP);
	g->relations  =  array_new(RG_Matrix, GRAPH_DEFAULT_RELATION_TYPE_CAP);

	GrB_Info info;
	UNUSED(info);

	GrB_Index n = Graph_RequiredMatrixDim(g);
	RG_Matrix_new(&g->node_labels, GrB_BOOL, n, n);
	RG_Matrix_new(&g->adjacency_matrix, GrB_BOOL, n, n);
	RG_Matrix_new(&g->adjacency_matrix->transposed, GrB_BOOL, n, n);
	RG_Matrix_new(&g->_zero_matrix, GrB_BOOL, n, n);

	// init graph statistics
	GraphStatistics_init(&g->stats);

	// initialize a read-write lock scoped to the individual graph
	_CreateRWLock(g);
	g->_writelocked = false;

	// force GraphBLAS updates and resize matrices to node count by default
	Graph_SetMatrixPolicy(g, SYNC_POLICY_FLUSH_RESIZE);

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

uint64_t Graph_LabeledNodeCount
(
	const Graph *g,
	int label_idx
) {
	return GraphStatistics_NodeCount(&g->stats, label_idx);
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

int Graph_GetNode
(
	const Graph *g,
	NodeID id,
	Node *n
) {
	ASSERT(g);
	ASSERT(n);
	n->entity = _Graph_GetEntity(g->nodes, id);
	n->id = id;
	return (n->entity != NULL);
}

int Graph_GetEdge
(
	const Graph *g,
	EdgeID id,
	Edge *e
) {
	ASSERT(g);
	ASSERT(e);
	ASSERT(id < g->edges->itemCap);

	e->id = id;
	e->entity = _Graph_GetEntity(g->edges, id);
	return (e->entity != NULL);
}

int Graph_GetEdgeRelation
(
	const Graph *g,
	Edge *e
) {
	ASSERT(g);
	ASSERT(e);

	GrB_Info info;
	int     rel         =  GRAPH_NO_RELATION;
	EdgeID  id          =  ENTITY_GET_ID(e);
	NodeID  srcNodeID   =  Edge_GetSrcNodeID(e);
	NodeID  destNodeID  =  Edge_GetDestNodeID(e);

	// search for relation mapping matrix M, where M[dest,src] == edge ID
	uint n = array_len(g->relations);
	for(uint i = 0; i < n; i++) {
		EdgeID edgeId = 0;
		RG_Matrix M = Graph_GetRelationMatrix(g, i, false);
		info = RG_Matrix_extractElement_UINT64(&edgeId, M, srcNodeID,
											   destNodeID);
		if(info != GrB_SUCCESS) continue;

		if(SINGLE_EDGE(edgeId)) {
			EdgeID curEdgeID = edgeId;
			if(curEdgeID == id) {
				Edge_SetRelationID(e, i);
				rel = i;
				break;
			}
		} else {
			// multiple edges exists between src and dest
			// see if given edge is one of them
			EdgeID *edges = (EdgeID *)(CLEAR_MSB(edgeId));
			int edge_count = array_len(edges);
			for(int j = 0; j < edge_count; j++) {
				if(edges[j] == id) {
					Edge_SetRelationID(e, i);
					rel = i;
					break;
				}
			}
		}
	}

	// we must be able to find edge relation
	ASSERT(rel != GRAPH_NO_RELATION);
	return rel;
}

void Graph_GetEdgesConnectingNodes
(
	const Graph *g,
	NodeID srcID,
	NodeID destID,
	int r,
	Edge **edges
) {
	ASSERT(g);
	ASSERT(edges);
	ASSERT(r < Graph_RelationTypeCount(g));

	// invalid relation type specified;
	// this can occur on multi-type traversals like:
	// MATCH ()-[:real_type|fake_type]->()
	if(r == GRAPH_UNKNOWN_RELATION) return;

#ifdef RG_DEBUG
	Node  srcNode   =  GE_NEW_NODE();
	Node  destNode  =  GE_NEW_NODE();
	ASSERT(Graph_GetNode(g, srcID, &srcNode));
	ASSERT(Graph_GetNode(g, destID, &destNode));
#endif

	if(r != GRAPH_NO_RELATION) {
		_Graph_GetEdgesConnectingNodes(g, srcID, destID, r, edges);
	} else {
		// relation type missing, scan through each edge type
		int relationCount = Graph_RelationTypeCount(g);
		for(int i = 0; i < relationCount; i++) {
			_Graph_GetEdgesConnectingNodes(g, srcID, destID, i, edges);
		}
	}
}

// label node id with each label in 'lbls'
static void _Graph_LabelNode
(
	Graph *g,
	NodeID id,
	int *lbls,
	uint lbl_count
) {
	ASSERT(g != NULL);
	ASSERT(lbls != NULL);
	ASSERT(lbl_count > 0);
	ASSERT(id != INVALID_ENTITY_ID);

	GrB_Info info;
	UNUSED(info);

	RG_Matrix nl = Graph_GetNodeLabelMatrix(g);
	for(uint i = 0; i < lbl_count; i++) {
		int l = lbls[i];
		// set matrix at position [id, id]
		RG_Matrix m = Graph_GetLabelMatrix(g, l);
		info = RG_Matrix_setElement_BOOL(m, id, id);
		ASSERT(info == GrB_SUCCESS);

		// map this label in this node's set of labels
		info = RG_Matrix_setElement_BOOL(nl, id, l);
		ASSERT(info == GrB_SUCCESS);

		// a node with 'label' has just been created, update statistics
		GraphStatistics_IncNodeCount(&g->stats, l, 1);
	}
}

void Graph_CreateNode
(
	Graph *g,
	Node *n,
	int *labels,
	uint label_count
) {
	ASSERT(g);
	ASSERT(n);
	ASSERT(label_count == 0 || (label_count > 0 && labels != NULL));

	NodeID id;
	Entity *en = DataBlock_AllocateItem(g->nodes, &id);

	n->id           =  id;
	n->entity       =  en;
	en->prop_count  =  0;
	en->properties  =  NULL;

	if(label_count > 0) _Graph_LabelNode(g, n->id, labels, label_count);
}

void Graph_FormConnection
(
	Graph *g,
	NodeID src,
	NodeID dest,
	EdgeID edge_id,
	int r
) {
	ASSERT(g != NULL);

	GrB_Info info;
	UNUSED(info);
	RG_Matrix  M    =  Graph_GetRelationMatrix(g, r, false);
	RG_Matrix  adj  =  Graph_GetAdjacencyMatrix(g, false);

	// rows represent source nodes, columns represent destination nodes
	info = RG_Matrix_setElement_BOOL(adj, src, dest);
	ASSERT(info == GrB_SUCCESS);

	info = RG_Matrix_setElement_UINT64(M, edge_id, src, dest);
	ASSERT(info == GrB_SUCCESS);

	// an edge of type r has just been created, update statistics
	GraphStatistics_IncEdgeCount(&g->stats, r, 1);
}

void Graph_CreateEdge
(
	Graph *g,
	NodeID src,
	NodeID dest,
	int r,
	Edge *e
) {
	ASSERT(g);
	ASSERT(r < Graph_RelationTypeCount(g));

#ifdef RG_DEBUG
	// make sure both src and destination nodes exists
	Node node = GE_NEW_NODE();
	ASSERT(Graph_GetNode(g, src, &node) == 1);
	ASSERT(Graph_GetNode(g, dest, &node) == 1);
#endif

	EdgeID id;
	Entity *en = DataBlock_AllocateItem(g->edges, &id);

	e->id           =  id;
	e->entity       =  en;
	e->srcNodeID    =  src;
	e->destNodeID   =  dest;
	e->relationID   =  r;
	en->prop_count  =  0;
	en->properties  =  NULL;

	Graph_FormConnection(g, src, dest, id, r);
}

// retrieves all either incoming or outgoing edges
// to/from given node N, depending on given direction
void Graph_GetNodeEdges
(
	const Graph *g,      // graph to collect edges from
	const Node *n,       // either source or destination node
	GRAPH_EDGE_DIR dir,  // edge direction ->, <-, <->
	int edgeType,        // relationship type
	Edge **edges         // [output] array of edges
) {
	ASSERT(g);
	ASSERT(n);
	ASSERT(edges);

	RG_MatrixTupleIter   it;
	RG_Matrix            M        =  NULL;
	RG_Matrix            TM       =  NULL;
	NodeID               srcID    =  ENTITY_GET_ID(n);
	NodeID               destID   =  INVALID_ENTITY_ID;
	EdgeID               edgeID   =  INVALID_ENTITY_ID;
	bool                 depleted =  false;

	if(edgeType == GRAPH_UNKNOWN_RELATION) return;

	bool outgoing = (dir == GRAPH_EDGE_DIR_OUTGOING ||
					 dir == GRAPH_EDGE_DIR_BOTH);

	bool incoming = (dir == GRAPH_EDGE_DIR_INCOMING ||
					 dir == GRAPH_EDGE_DIR_BOTH);

	// if a relationship type is specified,
	// retrieve the appropriate relation matrix
	// otherwise use the overall adjacency matrix
	M = Graph_GetRelationMatrix(g, edgeType, false);

	if(outgoing) {
		// construct an iterator to traverse over the source node row,
		// containing all outgoing edges
		RG_MatrixTupleIter_reuse(&it, M);
		RG_MatrixTupleIter_iterate_row(&it, srcID);
		while(RG_MatrixTupleIter_next(&it, NULL, &destID, &edgeID, &depleted) == GrB_SUCCESS) {
			if(depleted) break;

			// collect all edges (src)->(dest)
			if(edgeType != GRAPH_NO_RELATION) {
				_CollectEdgesFromEntry(g, srcID, destID, edgeType, edgeID, edges);
			} else {
				Graph_GetEdgesConnectingNodes(g, srcID, destID, edgeType, edges);
			}
		}
	}

	if(incoming) {
		// if a relationship type is specified, retrieve the appropriate
		// transposed relation matrix,
		// otherwise use the transposed adjacency matrix
		TM = Graph_GetRelationMatrix(g, edgeType, true);

		// construct an iterator to traverse over the source node row,
		// containing all incoming edges
		RG_MatrixTupleIter_reuse(&it, TM);
		RG_MatrixTupleIter_iterate_row(&it, srcID);

		while(RG_MatrixTupleIter_next(&it, NULL, &destID, NULL, &depleted) == GrB_SUCCESS) {
			if(depleted) break;
			RG_Matrix_extractElement_UINT64(&edgeID, M, destID, srcID);
			// collect all edges connecting destId to srcId
			if(edgeType != GRAPH_NO_RELATION) {
				_CollectEdgesFromEntry(g, destID, srcID, edgeType, edgeID, edges);
			} else {
				Graph_GetEdgesConnectingNodes(g, destID, srcID, edgeType, edges);
			}
		}
	}
}

// populate array of node's label IDs, return number of labels on node
uint Graph_GetNodeLabels
(
	const Graph *g,   // graph the node belongs to
	const Node *n,    // node to extract labels from
	LabelID *labels,  // array to populate with labels
	uint label_count  // size of labels array
) {
	// validate inputs
	ASSERT(g      != NULL);
	ASSERT(n      != NULL);
	ASSERT(labels != NULL);

	GrB_Info res;
	UNUSED(res);

	// GrB_Col_extract will iterate over the range of the output size
	RG_Matrix M = Graph_GetNodeLabelMatrix(g);

	RG_MatrixTupleIter iter;
	res = RG_MatrixTupleIter_reuse(&iter, M);
	ASSERT(res == GrB_SUCCESS);

	EntityID id = ENTITY_GET_ID(n);
	res = RG_MatrixTupleIter_iterate_row(&iter, id);
	ASSERT(res == GrB_SUCCESS);

	uint i = 0;
	bool depleted = false;

	for(; i < label_count; i++) {
		res = RG_MatrixTupleIter_next(&iter, NULL, labels + i, NULL, &depleted);
		ASSERT(res == GrB_SUCCESS);

		if(depleted) break;
	}

	return i;
}

// removes an edge from Graph and updates graph relevent matrices
int Graph_DeleteEdge
(
	Graph *g,
	Edge *e
) {
	ASSERT(g != NULL);
	ASSERT(e != NULL);

	uint64_t    x;
	RG_Matrix   R;
	RG_Matrix   M;
	GrB_Info    info;
	EdgeID      edge_id;
	int         r         =  Edge_GetRelationID(e);
	NodeID      src_id    =  Edge_GetSrcNodeID(e);
	NodeID      dest_id   =  Edge_GetDestNodeID(e);

	R = Graph_GetRelationMatrix(g, r, false);

	// test to see if edge exists
	info = RG_Matrix_extractElement_UINT64(&edge_id, R, src_id, dest_id);
	if(info != GrB_SUCCESS) return 0;

	// an edge of type r has just been deleted, update statistics
	GraphStatistics_DecEdgeCount(&g->stats, r, 1);

	// single edge of type R connecting src to dest, delete entry
	info = RG_Matrix_removeEntry(R, src_id, dest_id, ENTITY_GET_ID(e));
	ASSERT(info == GrB_SUCCESS);

	if(SINGLE_EDGE(edge_id)) {
		// see if source is connected to destination with additional edges
		bool connected = false;
		int relationCount = Graph_RelationTypeCount(g);
		for(int i = 0; i < relationCount; i++) {
			if(i == r) continue;
			M = Graph_GetRelationMatrix(g, i, false);
			info = RG_Matrix_extractElement_UINT64(&x, M, src_id, dest_id);
			if(info == GrB_SUCCESS) {
				connected = true;
				break;
			}
		}

		// there are no additional edges connecting source to destination
		// remove edge from THE adjacency matrix
		if(!connected) {
			M = Graph_GetAdjacencyMatrix(g, false);
			info = RG_Matrix_removeElement_BOOL(M, src_id, dest_id);
			ASSERT(info == GrB_SUCCESS);
		}
	}

	// free and remove edges from datablock.
	DataBlock_DeleteItem(g->edges, ENTITY_GET_ID(e));
	return 1;
}

inline bool Graph_EntityIsDeleted(Entity *e) {
	return DataBlock_ItemIsDeleted(e);
}

void Graph_DeleteNode
(
	Graph *g,
	Node *n
) {
	// assumption, node is completely detected,
	// there are no incoming nor outgoing edges
	// leading to / from node
	ASSERT(g != NULL);
	ASSERT(n != NULL);

	uint label_count;
	NODE_GET_LABELS(g, n, label_count);
	for(uint i = 0; i < label_count; i++) {
		int label_id = labels[i];
		RG_Matrix M = Graph_GetLabelMatrix(g, label_id);
		// clear label matrix at position node ID
		GrB_Info res = RG_Matrix_removeElement_BOOL(M, ENTITY_GET_ID(n),
													ENTITY_GET_ID(n));
		// update statistics
		GraphStatistics_DecNodeCount(&g->stats, label_id, 1);
	}

	DataBlock_DeleteItem(g->nodes, ENTITY_GET_ID(n));
}

static void _Graph_FreeRelationMatrices
(
	const Graph *g
) {
	uint relationCount = Graph_RelationTypeCount(g);
	for(uint i = 0; i < relationCount; i++) RG_Matrix_free(&g->relations[i]);
}

static void _BulkDeleteNodes
(
	Graph *g,
	Node *nodes,
	uint node_count,
	uint *node_deleted,
	uint *edge_deleted
) {
	ASSERT(g != NULL);
	ASSERT(nodes != NULL);
	ASSERT(g->_writelocked);
	ASSERT(node_count > 0);

	RG_Matrix adj; // adjacency matrix
	adj = Graph_GetAdjacencyMatrix(g, false);

	Node *distinct_nodes = array_new(Node, 1);
	Edge *edges = array_new(Edge, 1);

	// removing duplicates
#define is_id_lt(a, b) (ENTITY_GET_ID((a)) < ENTITY_GET_ID((b)))
	QSORT(Node, nodes, node_count, is_id_lt);

	for(uint i = 0; i < node_count; i++) {
		while(i < node_count - 1 && ENTITY_GET_ID(nodes + i) == ENTITY_GET_ID(nodes + i + 1)) i++;

		// skip nodes that have already been deleted
		if(!DataBlock_GetItem(g->nodes, ENTITY_GET_ID(nodes + i))) continue;
		array_append(distinct_nodes, *(nodes + i));
	}

	node_count = array_len(distinct_nodes);

	//--------------------------------------------------------------------------
	// collect edges to delete
	//--------------------------------------------------------------------------

	for(uint i = 0; i < node_count; i++) {
		Node *n = distinct_nodes + i;
		GrB_Index src;
		GrB_Index dest;
		NodeID ID = ENTITY_GET_ID(n);

		// collect edges
		Graph_GetNodeEdges(g, n, GRAPH_EDGE_DIR_BOTH, GRAPH_NO_RELATION, &edges);
	}

	//--------------------------------------------------------------------------
	// remove edges from matrices
	//--------------------------------------------------------------------------

	uint _edge_deleted = Graph_EdgeCount(g);

	int relation_count = Graph_RelationTypeCount(g);
	int edge_deletion_count[relation_count];
	memset(edge_deletion_count, 0, relation_count * sizeof(edge_deletion_count[0]));

	int edge_count = array_len(edges);

	// removing duplicates
	QSORT(Edge, edges, edge_count, is_id_lt);

	for(int i = 0; i < edge_count; i++) {
		// As long as current is the same as follows.
		while(i < edge_count - 1 && ENTITY_GET_ID(edges + i) == ENTITY_GET_ID(edges + i + 1)) i++;

		Edge       *e       =  edges + i;
		NodeID     src      =  Edge_GetSrcNodeID(e);
		NodeID     dest     =  Edge_GetDestNodeID(e);
		EdgeID     edge_id  =  ENTITY_GET_ID(e);
		RG_Matrix  R        =  Graph_GetRelationMatrix(g, e->relationID, false);

		RG_Matrix_removeElement_BOOL(adj, src, dest);
		RG_Matrix_removeElement_UINT64(R, src, dest);
		DataBlock_DeleteItem(g->edges, edge_id);
		edge_deletion_count[e->relationID]++;
	}

	for(int i = 0; i < relation_count; i++) {
		// multiple edges of type i has just been deleted, update statistics
		if(edge_deletion_count[i])
			GraphStatistics_DecEdgeCount(&g->stats, i, edge_deletion_count[i]);
	}

	*edge_deleted += _edge_deleted - Graph_EdgeCount(g);

	//--------------------------------------------------------------------------
	// remove nodes from label matrices
	//--------------------------------------------------------------------------

	uint _node_deleted = Graph_NodeCount(g);
	// all nodes marked for deletion are detected (no incoming/outgoing edges)
	RG_Matrix M = Graph_GetNodeLabelMatrix(g);
	int node_type_count = Graph_LabelTypeCount(g);
	for(int i = 0; i < node_count; i++) {
		Node *n = distinct_nodes + i;
		NodeID entity_id = ENTITY_GET_ID(n);
		uint label_count;
		NODE_GET_LABELS(g, n, label_count);

		for(int i = 0; i < label_count; i++) {
			RG_Matrix L = Graph_GetLabelMatrix(g, labels[i]);
			RG_Matrix_removeElement_BOOL(L, entity_id, entity_id);
			RG_Matrix_removeElement_BOOL(M, entity_id, i);
			// update statistics for label of deleted node
			GraphStatistics_DecNodeCount(&g->stats, labels[i], 1);
		}

		DataBlock_DeleteItem(g->nodes, entity_id);
	}

	// update deleted node count
	*node_deleted += _node_deleted - Graph_NodeCount(g);

	// clean up
	array_free(edges);
	array_free(distinct_nodes);
}

static void _BulkDeleteEdges(Graph *g, Edge *edges, size_t edge_count) {
	ASSERT(g);
	ASSERT(g->_writelocked);
	ASSERT(edges);
	ASSERT(edge_count > 0);

	int        relationCount  =  Graph_RelationTypeCount(g);
	GrB_Index  n              =  Graph_RequiredMatrixDim(g);
	RG_Matrix  adj            =  Graph_GetAdjacencyMatrix(g,  false);

	for(int i = 0; i < edge_count; i++) {
		Edge       *e       =  edges + i;
		int        r        =  Edge_GetRelationID(e);
		RG_Matrix  R        =  Graph_GetRelationMatrix(g, r, false);
		NodeID     src_id   =  Edge_GetSrcNodeID(e);
		NodeID     dest_id  =  Edge_GetDestNodeID(e);
		EdgeID     edge_id  =  ENTITY_GET_ID(e);

		GrB_Info info = RG_Matrix_removeEntry(R, src_id, dest_id, edge_id);

		// edge of type r has just been deleted, update statistics
		GraphStatistics_DecEdgeCount(&g->stats, r, 1);

		// free and remove edges from datablock
		DataBlock_DeleteItem(g->edges, edge_id);

		int j = 0;
		for(; j < relationCount; j++) {
			GrB_Index e;
			RG_Matrix r = Graph_GetRelationMatrix(g, j, false);
			info = RG_Matrix_extractElement_UINT64(&e, r, src_id, dest_id);
			if(info == GrB_SUCCESS) break;
		}

		// no additional connection between src to dest
		// remove entry from adj matrix
		if(j == relationCount) {
			RG_Matrix_removeElement_BOOL(adj, src_id, dest_id);
		}
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
#define is_id_lt(a, b) (ENTITY_GET_ID((a)) < ENTITY_GET_ID((b)))
		QSORT(Edge, edges, edge_count, is_id_lt);

		size_t uniqueIdx = 0;
		for(int i = 0; i < edge_count; i++) {
			// As long as current is the same as follows.
			while(i < edge_count - 1 && ENTITY_GET_ID(edges + i) == ENTITY_GET_ID(edges + i + 1)) i++;

			// skip edges that have already been deleted
			if(!DataBlock_GetItem(g->edges, ENTITY_GET_ID(edges + i))) continue;
			edges[uniqueIdx] = edges[i];
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

int Graph_AddLabel
(
	Graph *g
) {
	ASSERT(g != NULL);

	RG_Matrix m;
	GrB_Info info;
	size_t n = Graph_RequiredMatrixDim(g);
	RG_Matrix_new(&m, GrB_BOOL, n, n);

	array_append(g->labels, m);

	// adding a new label, update the stats structures to support it
	GraphStatistics_IntroduceLabel(&g->stats);

	int labelID = Graph_LabelTypeCount(g) - 1;
	return labelID;
}

int Graph_AddRelationType
(
	Graph *g
) {
	ASSERT(g);

	RG_Matrix m;
	size_t n = Graph_RequiredMatrixDim(g);

	RG_Matrix_new(&m, GrB_UINT64, n, n);

	array_append(g->relations, m);

	// adding a new relationship type, update the stats structures to support it
	GraphStatistics_IntroduceRelationship(&g->stats);

	int relationID = Graph_RelationTypeCount(g) - 1;
	return relationID;
}

RG_Matrix Graph_GetLabelMatrix
(
	const Graph *g,
	int label_idx
) {
	ASSERT(g != NULL);
	ASSERT(label_idx < (int)array_len(g->labels));

	// return zero matrix if label_idx is out of range
	if(label_idx < 0) return Graph_GetZeroMatrix(g);

	RG_Matrix m = g->labels[label_idx];
	g->SynchronizeMatrix(g, m);
	return m;
}

RG_Matrix Graph_GetRelationMatrix
(
	const Graph *g,
	int relation_idx,
	bool transposed
) {
	ASSERT(g);
	ASSERT(relation_idx == GRAPH_NO_RELATION ||
		   relation_idx < Graph_RelationTypeCount(g));

	RG_Matrix m = GrB_NULL;

	if(relation_idx == GRAPH_NO_RELATION) m = g->adjacency_matrix;
	else m = g->relations[relation_idx];

	g->SynchronizeMatrix(g, m);

	if(transposed) m = RG_Matrix_getTranspose(m);

	return m;
}

RG_Matrix Graph_GetAdjacencyMatrix
(
	const Graph *g,
	bool transposed
) {
	return Graph_GetRelationMatrix(g, GRAPH_NO_RELATION, transposed);
}

// returns true if relationship matrix 'r' contains multi-edge entries,
// false otherwise
bool Graph_RelationshipContainsMultiEdge
(
	const Graph *g,
	int r,
	bool transpose
) {
	ASSERT(Graph_RelationTypeCount(g) > r);
	GrB_Index nvals;
	// A relationship matrix contains multi-edge if nvals < number of edges with type r.
	RG_Matrix R = Graph_GetRelationMatrix(g, r, transpose);
	RG_Matrix_nvals(&nvals, R);

	return (Graph_RelationEdgeCount(g, r) > nvals);
}

RG_Matrix Graph_GetNodeLabelMatrix(const Graph *g) {
	ASSERT(g != NULL);

	RG_Matrix m = g->node_labels;

	g->SynchronizeMatrix(g, m);

	return m;
}

RG_Matrix Graph_GetZeroMatrix
(
	const Graph *g
) {
	RG_Matrix z = g->_zero_matrix;
	_MatrixResizeToCapacity(g, z);

#if RG_DEBUG
	// make sure zero matrix is indeed empty
	GrB_Index nvals;
	RG_Matrix_nvals(&nvals, z);
	ASSERT(nvals == 0);
#endif

	return z;
}

void Graph_Free(Graph *g) {
	ASSERT(g);
	// free matrices
	Entity *en;
	DataBlockIterator *it;
	RG_Matrix_free(&g->_zero_matrix);
	RG_Matrix_free(&g->adjacency_matrix);

	_Graph_FreeRelationMatrices(g);
	array_free(g->relations);
	GraphStatistics_FreeInternals(&g->stats);

	uint32_t labelCount = array_len(g->labels);
	for(int i = 0; i < labelCount; i++) RG_Matrix_free(&g->labels[i]);
	array_free(g->labels);
	RG_Matrix_free(&g->node_labels);

	it = Graph_ScanNodes(g);
	while((en = (Entity *)DataBlockIterator_Next(it, NULL)) != NULL) {
		FreeEntity(en);
	}
	DataBlockIterator_Free(it);

	it = Graph_ScanEdges(g);
	while((en = DataBlockIterator_Next(it, NULL)) != NULL) FreeEntity(en);
	DataBlockIterator_Free(it);

	// free blocks
	DataBlock_Free(g->nodes);
	DataBlock_Free(g->edges);

	int res;
	UNUSED(res);

	if(g->_writelocked) Graph_ReleaseLock(g);
	res = pthread_rwlock_destroy(&g->_rwlock);
	ASSERT(res == 0);

	rm_free(g);
}

