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
#include "../algorithms/LAGraph_bfs_both.h"

// MATCH (a:User {id: 1}) CALL algo.bfs(a, 0, 'MANAGES') YIELD nodes, edges
typedef struct {
	Graph *g;                       // Graph.
	GrB_Index n;                    // Total number of results.
	GrB_Index *node_ids;            // Array of connected nodes.
	GrB_Vector parents;             // Vector associating each node in the BFS tree with its parent.
	int reltype_id;                 // ID of relationship matrix to traverse.
	int nodes_output_idx;           // Offset of nodes array in outputs
	int edges_output_idx;           // Offset of edges array in outputs
	SIValue *output;                // Array with a maximum of 4 entries: ["nodes", nodes, "edges", edges].
	bool depleted;                  // True if BFS has already been performed for this node.
} BFSContext;

static ProcedureResult Proc_BFS_Invoke(ProcedureCtx *ctx, const SIValue *args) {
	if(array_len((SIValue *)args) != 3) return PROCEDURE_ERR;
	if(SI_TYPE(args[0]) != T_NODE                 ||   // Source node.
	   SI_TYPE(args[1]) != T_INT64                ||   // Max level to iterate to, unlimited if 0.
	   !(SI_TYPE(args[2]) & (T_NULL | T_STRING)))      // Relationship type to traverse if not NULL.
		return PROCEDURE_ERR;

	BFSContext *pdata = ctx->privateData;
	Node *source_node = args[0].ptrval;
	GrB_Index source_id = ENTITY_GET_ID(source_node);
	int64_t max_level = args[1].longval;
	/* The BFS algorithm uses a level of 1 to indicate the source node. If this value is not
	 * zero (unlimited), increment it by 1 to make level 1 indicate the source's direct neighbors. */
	if(max_level > 0) max_level++;
	const char *reltype = SIValue_IsNull(args[2]) ? NULL : args[2].stringval;
	bool collect_edges = (pdata->edges_output_idx >= 0);

	GraphContext *gc = QueryCtx_GetGraphCtx();
	// Get edge matrix and transpose matrix, if available.
	GrB_Matrix R;
	GrB_Matrix TR;
	if(reltype == NULL) {
		R = Graph_GetAdjacencyMatrix(gc->g);
		TR = Graph_GetTransposedAdjacencyMatrix(gc->g);
	} else {
		Schema *s = GraphContext_GetSchema(gc, reltype, SCHEMA_EDGE);
		if(!s) return PROCEDURE_OK; // Failed to find schema, first step will return NULL.
		pdata->reltype_id = s->id;
		R = Graph_GetRelationMatrix(gc->g, s->id);
		if(Config_MaintainTranspose()) TR = Graph_GetTransposedRelationMatrix(gc->g, s->id);
		else TR = GrB_NULL;
	}

	GrB_Vector V;         // Vector of results
	GrB_Vector PI = NULL; // Vector backtracking results to their parents.
	GrB_Vector *PI_ptr = &PI;
	/* If we're not collecting edges, pass a NULL parent pointer so that the algorithm will
	 * not perform unnecessary work. */
	if(!collect_edges) PI_ptr = GrB_NULL;
	GrB_Info res = LAGraph_bfs_both(&V, PI_ptr, R, TR, source_id, max_level, false);
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
	pdata->n = nvals;

	// Retrieve all tuples representing connected nodes.
	GrB_Index *node_ids = rm_malloc(nvals * sizeof(GrB_Index));
	GrB_Vector_extractTuples_INT64(node_ids, GrB_NULL, &nvals, V);
	pdata->node_ids = node_ids;
	pdata->parents = PI;

	GrB_Vector_free(&V);
	return PROCEDURE_OK;
}

static SIValue *Proc_BFS_Step(ProcedureCtx *ctx) {
	ASSERT(ctx->privateData);

	BFSContext *pdata = (BFSContext *)ctx->privateData;

	// Return NULL if the BFS for thise source has already been emitted or there are no connected nodes.
	if(pdata->depleted || pdata->n == 0) return NULL;

	// Build arrays for the outputs the user has requested.
	if(pdata->nodes_output_idx >= 0) pdata->output[pdata->nodes_output_idx] = SI_Array(pdata->n);
	if(pdata->edges_output_idx >= 0) pdata->output[pdata->edges_output_idx] = SI_Array(pdata->n);

	Edge *edges = array_new(Edge, 1);
	for(int i = 0; i < pdata->n; i ++) {
		// Get the reached node.
		NodeID node_id = pdata->node_ids[i];

		if(pdata->nodes_output_idx >= 0) {
			// Append each reachable node to the nodes output array.
			Node n = GE_NEW_NODE();
			Graph_GetNode(pdata->g, node_id, &n);
			SIArray_Append(&pdata->output[pdata->nodes_output_idx], SI_Node(&n));
		}

		array_clear(edges);
		if(pdata->edges_output_idx >= 0) {
			GrB_Index parent_id;
			// Find the parent of the reached node.
			GrB_Info res = GrB_Vector_extractElement(&parent_id, pdata->parents, node_id);
			ASSERT(res == GrB_SUCCESS);
			parent_id --; // Decrement the parent ID by 1 to correct 1-indexing.
			// Retrieve edges connecting the parent node to the current node.
			Graph_GetEdgesConnectingNodes(pdata->g, parent_id, node_id, pdata->reltype_id, &edges);
			// Append one edge to the edges output array.
			SIArray_Append(&pdata->output[pdata->edges_output_idx], SI_Edge(&edges[0]));
		}
	}
	array_free(edges);

	pdata->depleted = true; // Mark that this node has been mapped.

	return pdata->output;
}

static ProcedureResult Proc_BFS_Free(ProcedureCtx *ctx) {
	// Clean up.
	BFSContext *pdata = ctx->privateData;
	if(pdata->output) array_free(pdata->output);
	if(pdata->node_ids) rm_free(pdata->node_ids);
	if(pdata->parents) GrB_Vector_free(&pdata->parents);
	rm_free(ctx->privateData);

	return PROCEDURE_OK;
}

static BFSContext *_Proc_BFS_BuildContext(const char **yields, ProcedureOutput ***outputs) {
	// Set up the BFS context.
	BFSContext *pdata = rm_calloc(1, sizeof(BFSContext));
	pdata->g = QueryCtx_GetGraph();
	pdata->n = 0;
	pdata->depleted = false;
	pdata->nodes_output_idx = -1;
	pdata->edges_output_idx = -1;
	pdata->reltype_id = GRAPH_NO_RELATION;

	uint yield_count = array_len(yields);
	pdata->output = array_new(SIValue, yield_count * 2);
	for(uint i = 0; i < yield_count; i++) {
		ProcedureOutput *output = rm_malloc(sizeof(ProcedureOutput));
		// TODO Improve output logic to remove redundancy of internal and external outputs.
		if(!strcasecmp(yields[i], "nodes")) {
			// Internal outputs array.
			pdata->output = array_append(pdata->output, SI_ConstStringVal("nodes"));
			pdata->nodes_output_idx = array_len(pdata->output);
			pdata->output = array_append(pdata->output, SI_NullVal()); // Place holder.

			// External outputs array.
			output->name = "nodes";
			output->type = T_ARRAY;
		} else if(!strcasecmp(yields[i], "edges")) {
			pdata->output = array_append(pdata->output, SI_ConstStringVal("edges"));
			pdata->edges_output_idx = array_len(pdata->output);
			pdata->output = array_append(pdata->output, SI_NullVal()); // Place holder.

			output->name = "edges";
			output->type = T_ARRAY;
		} else {
			// Unreachable, yield strings have been validated in the AST.
			ASSERT(false);
		}
		*outputs = array_append(*outputs, output);
	}
	return pdata;
}

ProcedureCtx *Proc_BFS_Ctx(AR_ExpNode **args, const char **yields) {
	bool default_yields = (yields == NULL);
	if(default_yields) {
		// If the user did not add an explicit YIELD, use the default yields - both nodes and edges.
		int yield_count = 2;
		yields = array_new(const char *, yield_count);
		yields = array_append(yields, "nodes");
		yields = array_append(yields, "edges");
	}
	ProcedureOutput **outputs = array_new(ProcedureOutput *, array_len(yields));
	// Populate the BFSContext held within this procedure.
	void *privdata = _Proc_BFS_BuildContext(yields, &outputs);
	if(default_yields) array_free(yields);

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

