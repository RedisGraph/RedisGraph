/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "proc_bfs.h"
#include "../value.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../util/rmalloc.h"
#include "../datatypes/array.h"
#include "../graph/graphcontext.h"
#include "../configuration/config.h"
#include "../algorithms/LAGraph/LAGraph_bfs_pushpull.h"

// The BFS procedure performs a single source BFS scan
// it's inputs are:
// 1. source node to traverse from
// 2. depth, how deep should the procedure traverse (0 no limit)
// 3. relationship type to traverse, (NULL for edge type agnostic)
//
// output:
// 1. nodes - an array of reachable nodes
// 2. edges- an array of edges traversed
//
// MATCH (a:User {id: 1}) CALL algo.bfs(a, 0, 'MANAGES') YIELD nodes, edges

typedef struct {
	Graph *g;              // graph scanned
	GrB_Index n;           // total number of results
	bool depleted;         // true if BFS has already been performed for this node
	int reltype_id;        // iD of relationship matrix to traverse
	SIValue *output;       // array with a maximum of 4 entries: ["nodes", nodes, "edges", edges]
	SIValue *yield_nodes;  // yield reachable nodes
	SIValue *yield_edges;  // yield edges traversed
	GrB_Vector nodes;      // vector of reachable nodes
	GrB_Vector parents;    // vector associating each node in the BFS tree with its parent
} BFSCtx;

static void _process_yield
(
	BFSCtx *ctx,
	const char **yield
) {
	ctx->yield_nodes = NULL;
	ctx->yield_edges = NULL;

	int idx = 0;
	for(uint i = 0; i < array_len(yield); i++) {
		if(strcasecmp("nodes", yield[i]) == 0) {
			ctx->yield_nodes = ctx->output + idx;
			idx ++;
			continue;
		}

		if(strcasecmp("edges", yield[i]) == 0) {
			ctx->yield_edges = ctx->output + idx;
			idx ++;
			continue;
		}
	}
}

static ProcedureResult Proc_BFS_Invoke
(
	ProcedureCtx *ctx,
	const SIValue *args,
	const char **yield
) {
	// validate inputs
	ASSERT(ctx   !=  NULL);
	ASSERT(args  !=  NULL);

	if(array_len((SIValue *)args) != 3) return PROCEDURE_ERR;
	if(SI_TYPE(args[0]) != T_NODE                 ||   // source node
	   SI_TYPE(args[1]) != T_INT64                ||   // max level to iterate to, unlimited if 0
	   !(SI_TYPE(args[2]) & (T_NULL | T_STRING)))      // relationship type to traverse if not NULL
		return PROCEDURE_ERR;

	BFSCtx *bfs_ctx = ctx->privateData;
	_process_yield(bfs_ctx, yield);

	//--------------------------------------------------------------------------
	// Process inputs
	//--------------------------------------------------------------------------

	Node *source_node = args[0].ptrval;
	int64_t max_level = args[1].longval;
	const char *reltype = SIValue_IsNull(args[2]) ? NULL : args[2].stringval;

	GrB_Index src_id = ENTITY_GET_ID(source_node);

	// Get edge matrix and transpose matrix, if available.
	GrB_Matrix    R    =  NULL;
	GrB_Matrix    TR   =  NULL;
	GraphContext  *gc  =  QueryCtx_GetGraphCtx();

	if(reltype == NULL) {
		assert(false && "export isn't thread safe, modified D+ and D-");
		RG_Matrix_export(&R, Graph_GetAdjacencyMatrix(gc->g, false));
		RG_Matrix_export(&TR, Graph_GetAdjacencyMatrix(gc->g, true));
	} else {
		Schema *s = GraphContext_GetSchema(gc, reltype, SCHEMA_EDGE);
		// failed to find schema, first step will return NULL
		if(!s) return PROCEDURE_OK;

		bfs_ctx->reltype_id = s->id;
		assert(false && "export isn't thread safe, modified D+ and D-");
		RG_Matrix_export(&R, Graph_GetRelationMatrix(gc->g, s->id, false));
		RG_Matrix_export(&TR, Graph_GetRelationMatrix(gc->g, s->id, true));
	}

	// if we're not collecting edges, pass a NULL parent pointer
	// so that the algorithm will not perform unnecessary work
	GrB_Vector V = GrB_NULL;  // vector of results
	GrB_Vector PI = GrB_NULL; // vector backtracking results to their parents
	GrB_Vector *pPI = &PI;
	if(!bfs_ctx->yield_edges) pPI = NULL;
	GrB_Info res = LG_BreadthFirstSearch_SSGrB(&V, pPI, R, TR, src_id, NULL, max_level);
	ASSERT(res == GrB_SUCCESS);

	// remove all values with a level less than or equal to 1
	// values of 0 are not connected to the source, and values of 1 are the source
	GxB_Scalar thunk;
	GxB_Scalar_new(&thunk, GrB_UINT64);
	GxB_Scalar_setElement_UINT64(thunk, 0);
	GxB_Vector_select(V, GrB_NULL, GrB_NULL, GxB_GT_THUNK, V, thunk, GrB_NULL);
	GxB_Scalar_free(&thunk);

	// get number of entries
	GrB_Index nvals;
	GrB_Vector_nvals(&nvals, V);

	bfs_ctx->n = nvals;
	bfs_ctx->nodes = V;
	bfs_ctx->parents = PI;

	// matrix iterator requires matrix format to be sparse
	// to avoid future conversion from HYPER-SPARSE, BITMAP, FULL to SPARSE
	// we set matrix format at creation time
	//
	// TODO: we do support HYPER-SPARSE and ISO
	// check each vector format and only switch sparsity if needed be
	// i.e. current format isn't SPARSE nor HYPER-SPARSE
	GxB_Vector_Option_set(bfs_ctx->nodes, GxB_SPARSITY_CONTROL, GxB_SPARSE);
	GxB_Vector_Option_set(bfs_ctx->parents, GxB_SPARSITY_CONTROL, GxB_SPARSE);

	GrB_Matrix_free(&R);
	GrB_Matrix_free(&TR);

	return PROCEDURE_OK;
}

static SIValue *Proc_BFS_Step
(
	ProcedureCtx *ctx
) {
	ASSERT(ctx->privateData);

	BFSCtx *bfs_ctx = (BFSCtx *)ctx->privateData;

	// return NULL if the BFS for this source has already been emitted
	// or there are no connected nodes
	if(bfs_ctx->depleted || bfs_ctx->n == 0) return NULL;

	bool yield_nodes = (bfs_ctx->yield_nodes != NULL);
	bool yield_edges = (bfs_ctx->yield_edges != NULL);

	// build arrays for the outputs the user has requested
	uint n = bfs_ctx->n;
	SIValue nodes, edges;
	if(yield_nodes) nodes = SI_Array(n);
	if(yield_edges) edges = SI_Array(n);
	Edge *edge = array_new(Edge, 1);

	// setup result iterator
	NodeID               id;
	GrB_Info             res;
	bool                 depleted;
	GxB_MatrixTupleIter  iter;

	UNUSED(res);
	res = GxB_MatrixTupleIter_reuse(&iter, (GrB_Matrix)bfs_ctx->nodes);
	ASSERT(res == GrB_SUCCESS);
	res = GxB_MatrixTupleIter_next(&iter, NULL, &id, NULL, &depleted);
	ASSERT(res == GrB_SUCCESS);

	while(!depleted) {
		// get the reached node
		if(yield_nodes) {
			// append each reachable node to the nodes output array
			Node n = GE_NEW_NODE();
			Graph_GetNode(bfs_ctx->g, id, &n);
			SIArray_Append(&nodes, SI_Node(&n));
		}

		array_clear(edge);
		if(yield_edges) {
			GrB_Index parent_id;
			// find the parent of the reached node
			GrB_Info res = GrB_Vector_extractElement(&parent_id,
					bfs_ctx->parents, id);
			ASSERT(res == GrB_SUCCESS);
			// retrieve edges connecting the parent node to the current node
			// TODO: we only require a single edge
			// `Graph_GetEdgesConnectingNodes` can return multiple edges
			Graph_GetEdgesConnectingNodes(bfs_ctx->g, parent_id, id, bfs_ctx->reltype_id, &edge);
			// append one edge to the edges output array
			SIArray_Append(&edges, SI_Edge(edge));
		}

		res = GxB_MatrixTupleIter_next(&iter, NULL, &id, NULL, &depleted);
		ASSERT(res == GrB_SUCCESS);
	}

	bfs_ctx->depleted = depleted;

	// populate output
	if(yield_nodes) *bfs_ctx->yield_nodes = nodes;
	if(yield_edges) *bfs_ctx->yield_edges = edges;

	// clean up
	array_free(edge);

	return bfs_ctx->output;
}

static ProcedureResult Proc_BFS_Free
(
	ProcedureCtx *ctx
) {
	ASSERT(ctx != NULL);

	// free private data
	BFSCtx *pdata = ctx->privateData;

	if(pdata->output   !=  NULL)  array_free(pdata->output);
	if(pdata->nodes    !=  NULL)  GrB_Vector_free(&pdata->nodes);
	if(pdata->parents  !=  NULL)  GrB_Vector_free(&pdata->parents);

	rm_free(ctx->privateData);

	return PROCEDURE_OK;
}

static BFSCtx *_Build_Private_Data() {
	// set up the BFS context
	BFSCtx *pdata = rm_calloc(1, sizeof(BFSCtx));

	pdata->n            =  0;
	pdata->g            =  QueryCtx_GetGraph();
	pdata->nodes        =  GrB_NULL;
	pdata->output       =  array_new(SIValue, 2);
	pdata->parents      =  GrB_NULL;
	pdata->depleted     =  false;
	pdata->reltype_id   =  GRAPH_NO_RELATION;
	pdata->yield_nodes  =  NULL;
	pdata->yield_edges  =  NULL;

	return pdata;
}

ProcedureCtx *Proc_BFS_Ctx() {
	// Construct procedure private data.
	void *privdata = _Build_Private_Data();

	// Declare possible outputs.
	ProcedureOutput *outputs = array_new(ProcedureOutput, 2);
	ProcedureOutput out_nodes = {.name = "nodes", .type = T_ARRAY};
	ProcedureOutput out_edges = {.name = "edges", .type = T_ARRAY};
	array_append(outputs, out_nodes);
	array_append(outputs, out_edges);

	ProcedureCtx *ctx = ProcCtxNew("algo.BFS",
								   3,
								   outputs,
								   Proc_BFS_Step,
								   Proc_BFS_Invoke,
								   Proc_BFS_Free,
								   privdata,
								   true);
	return ctx;
}

