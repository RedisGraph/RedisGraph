/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "proc_bfs.h"
#include "../RG.h"
#include "../value.h"
#include "../config.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../util/rmalloc.h"
#include "../datatypes/array.h"
#include "../graph/graphcontext.h"
#include "../algorithms/LAGraph_bfs_pushpull.h"

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
	Graph *g;                       // Graph scanned.
	GrB_Index n;                    // Total number of results.
	bool depleted;                  // True if BFS has already been performed for this node.
	int reltype_id;                 // ID of relationship matrix to traverse.
	SIValue *output;                // Array with a maximum of 4 entries: ["nodes", nodes, "edges", edges].
	bool yield_nodes;               // Return reachable nodes.
	bool yield_edges;               // Return edges traversed.
	GrB_Vector nodes;               // Vector of reachable nodes.
	GrB_Vector parents;             // Vector associating each node in the BFS tree with its parent.
	int nodes_output_idx;           // Offset of nodes array in outputs
	int edges_output_idx;           // Offset of edges array in outputs
} BFSCtx;

static void _process_yield(BFSCtx *ctx, const char **yield) {
	bool yield_nodes = false;
	bool yield_edges = false;

	if(yield != NULL) {
		for(uint i = 0; i < array_len(yield); i++) {
			if(strcasecmp("nodes", yield[i]) == 0) {
				yield_nodes = true;
				continue;
			}
			if(strcasecmp("edges", yield[i]) == 0) {
				yield_edges = true;
				continue;
			}
		}
	} else {
		/* User did not add an explicit YIELD
		 * use the default yields - both nodes and edges. */
		yield_nodes = true;
		yield_edges = true;
	}

	ctx->yield_nodes = yield_nodes;
	ctx->yield_edges = yield_edges;

	if(yield_nodes) {
		ctx->output = array_append(ctx->output, SI_ConstStringVal("nodes"));
		ctx->nodes_output_idx = array_len(ctx->output);
		ctx->output = array_append(ctx->output, SI_NullVal()); // Place holder.
	}

	if(yield_edges) {
		ctx->output = array_append(ctx->output, SI_ConstStringVal("edges"));
		ctx->edges_output_idx = array_len(ctx->output);
		ctx->output = array_append(ctx->output, SI_NullVal()); // Place holder.
	}
}

static ProcedureResult Proc_BFS_Invoke(ProcedureCtx *ctx,
		const SIValue *args, const char **yield) {
	// Validate inputs
	ASSERT(ctx != NULL);
	ASSERT(args != NULL);

	if(array_len((SIValue *)args) != 3) return PROCEDURE_ERR;
	if(SI_TYPE(args[0]) != T_NODE                 ||   // Source node.
	   SI_TYPE(args[1]) != T_INT64                ||   // Max level to iterate to, unlimited if 0.
	   !(SI_TYPE(args[2]) & (T_NULL | T_STRING)))      // Relationship type to traverse if not NULL.
		return PROCEDURE_ERR;

	BFSCtx *bfs_ctx = ctx->privateData;
	_process_yield(bfs_ctx, yield);

	//--------------------------------------------------------------------------
	// Process inputs
	//--------------------------------------------------------------------------

	Node *source_node = args[0].ptrval;
	int64_t max_level = args[1].longval;
	const char *reltype = SIValue_IsNull(args[2]) ? NULL : args[2].stringval;

	/* The BFS algorithm uses a level of 1 to indicate the source node.
	 * If this value is not zero (unlimited), increment it by 1
	 * to make level 1 indicate the source's direct neighbors. */
	if(max_level > 0) max_level++;
	GrB_Index src_id = ENTITY_GET_ID(source_node);

	// Get edge matrix and transpose matrix, if available.
	GrB_Matrix R;
	GrB_Matrix TR;
	GraphContext *gc = QueryCtx_GetGraphCtx();
	if(reltype == NULL) {
		R = Graph_GetAdjacencyMatrix(gc->g);
		TR = Graph_GetTransposedAdjacencyMatrix(gc->g);
	} else {
		Schema *s = GraphContext_GetSchema(gc, reltype, SCHEMA_EDGE);
		if(!s) return PROCEDURE_OK; // Failed to find schema, first step will return NULL.
		bfs_ctx->reltype_id = s->id;
		R = Graph_GetRelationMatrix(gc->g, s->id);
		if(Config_MaintainTranspose()) TR = Graph_GetTransposedRelationMatrix(gc->g, s->id);
		else TR = GrB_NULL;
	}

	/* If we're not collecting edges, pass a NULL parent pointer
	 * so that the algorithm will not perform unnecessary work. */
	GrB_Vector V = GrB_NULL;  // Vector of results
	GrB_Vector PI = GrB_NULL; // Vector backtracking results to their parents.
	GrB_Vector *pPI = &PI;
	if(!bfs_ctx->yield_edges) pPI = NULL;
	GrB_Info res = LAGraph_bfs_pushpull(&V, pPI, R, TR, src_id, max_level, true);
	ASSERT(res == GrB_SUCCESS);

	/* Remove all values with a level less than or equal to 1.
	 * Values of 0 are not connected to the source, and values of 1 are the source. */
	GxB_Scalar thunk;
	GxB_Scalar_new(&thunk, GrB_UINT64);
	GxB_Scalar_setElement_UINT64(thunk, 1);
	GxB_Vector_select(V, GrB_NULL, GrB_NULL, GxB_GT_THUNK, V, thunk, GrB_NULL);
	GxB_Scalar_free(&thunk);

	// Get number of entries.
	GrB_Index nvals;
	GrB_Vector_nvals(&nvals, V);

	bfs_ctx->n = nvals;
	bfs_ctx->nodes = V;
	bfs_ctx->parents = PI;

	return PROCEDURE_OK;
}

static SIValue *Proc_BFS_Step(ProcedureCtx *ctx) {
	ASSERT(ctx->privateData);

	BFSCtx *bfs_ctx = (BFSCtx *)ctx->privateData;

	// Return NULL if the BFS for this source has already been emitted or there are no connected nodes.
	if(bfs_ctx->depleted || bfs_ctx->n == 0) return NULL;

	// Build arrays for the outputs the user has requested.
	uint n = bfs_ctx->n;
	SIValue nodes, edges;
	if(bfs_ctx->yield_nodes) nodes = SI_Array(n);
	if(bfs_ctx->yield_edges) edges = SI_Array(n);
	Edge *edge = array_new(Edge, 1);

	// Setup result iterator
	NodeID id;
	GrB_Info res;
	bool depleted;
	GxB_MatrixTupleIter *iter;

	UNUSED(res);
	res = GxB_MatrixTupleIter_new(&iter, (GrB_Matrix)bfs_ctx->nodes);
	ASSERT(res == GrB_SUCCESS);
	res = GxB_MatrixTupleIter_next(iter, NULL, &id, &depleted);
	ASSERT(res == GrB_SUCCESS);

	while(!depleted) {
		// Get the reached node.
		if(bfs_ctx->yield_nodes) {
			// Append each reachable node to the nodes output array.
			Node n = GE_NEW_NODE();
			Graph_GetNode(bfs_ctx->g, id, &n);
			SIArray_Append(&nodes, SI_Node(&n));
		}

		array_clear(edge);
		if(bfs_ctx->yield_edges) {
			GrB_Index parent_id;
			// Find the parent of the reached node.
			GrB_Info res = GrB_Vector_extractElement(&parent_id, bfs_ctx->parents, id);
			ASSERT(res == GrB_SUCCESS);
			parent_id --; // Decrement the parent ID by 1 to correct 1-indexing.
			// Retrieve edges connecting the parent node to the current node.
			Graph_GetEdgesConnectingNodes(bfs_ctx->g, parent_id, id, bfs_ctx->reltype_id, &edge);
			// Append one edge to the edges output array.
			SIArray_Append(&edges, SI_Edge(edge));
		}

		res = GxB_MatrixTupleIter_next(iter, NULL, &id, &depleted);
		ASSERT(res == GrB_SUCCESS);
	}

	bfs_ctx->depleted = depleted; // Mark that this node has been mapped.

	// Populate output.
	if(bfs_ctx->yield_nodes) bfs_ctx->output[bfs_ctx->nodes_output_idx] = nodes;
	if(bfs_ctx->yield_edges) bfs_ctx->output[bfs_ctx->edges_output_idx] = edges;

	// Clean up.
	array_free(edge);
	GxB_MatrixTupleIter_free(iter);

	return bfs_ctx->output;
}

static ProcedureResult Proc_BFS_Free(ProcedureCtx *ctx) {
	ASSERT(ctx != NULL);
	// Free private data.
	BFSCtx *pdata = ctx->privateData;
	if(pdata->output != NULL) array_free(pdata->output);
	if(pdata->nodes != NULL) GrB_Vector_free(&pdata->nodes);
	if(pdata->parents != NULL) GrB_Vector_free(&pdata->parents);
	rm_free(ctx->privateData);

	return PROCEDURE_OK;
}

static BFSCtx *_Build_Private_Data() {
	// Set up the BFS context.
	BFSCtx *pdata = rm_calloc(1, sizeof(BFSCtx));
	pdata->n = 0;
	pdata->nodes = GrB_NULL;
	pdata->depleted = false;
	pdata->parents = GrB_NULL;
	pdata->yield_nodes = false;
	pdata->yield_edges = false;
	pdata->nodes_output_idx = -1;
	pdata->edges_output_idx = -1;
	pdata->g = QueryCtx_GetGraph();
	pdata->reltype_id = GRAPH_NO_RELATION;
	pdata->output = array_new(SIValue, 4);
	return pdata;
}

ProcedureCtx *Proc_BFS_Ctx() {
	// Construct procedure private data.
	void *privdata = _Build_Private_Data();

	// Declare possible outputs.
	ProcedureOutput *outputs = array_new(ProcedureOutput, 2);
	ProcedureOutput out_nodes = {name: "nodes", type: T_ARRAY};
	ProcedureOutput out_edges = {name: "edges", type: T_ARRAY};
	outputs = array_append(outputs, out_nodes);
	outputs = array_append(outputs, out_edges);

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

