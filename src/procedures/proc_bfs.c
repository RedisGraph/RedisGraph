/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "proc_bfs.h"
#include "../value.h"
#include "../config.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../util/rmalloc.h"
#include "../datatypes/array.h"
#include "../graph/graphcontext.h"
#include "../algorithms/LAGraph_bfs_both.h"

// MATCH (a:User {id: 1}) CALL algo.bfs(a, 0, 'MANAGES', true) YIELD start_node, nodes, edges
typedef struct {
	Graph *g;                       // Graph.
	GrB_Index n;                    // Total number of results.
	GrB_Index *node_ids;            // Array of connected nodes.
	GrB_Vector parents;             // Vector associating each node in the BFS tree with its parent.
	int reltype_id;                 // ID of relationship matrix to traverse.
	int start_node_output_idx;      // Offset of start node in outputs
	int nodes_output_idx;           // Offset of nodes array in outputs
	int edges_output_idx;           // Offset of edges array in outputs
	SIValue *output;                // Array with a maximum of 6 entries: ["node", node, "level", level, "path", path].
	bool depleted;
} BFSContext;

static ProcedureResult Proc_BFS_Invoke(ProcedureCtx *ctx, const SIValue *args) {
	if(array_len((SIValue *)args) != 4) return PROCEDURE_ERR;
	if(SI_TYPE(args[0]) != T_NODE                 ||   // Source node.
	   SI_TYPE(args[1]) != T_INT64                ||   // Max level to iterate to, unlimited if 0.
	   !(SI_TYPE(args[2]) & (T_NULL | T_STRING))  ||   // Relationship type to traverse if not NULL.
	   SI_TYPE(args[3]) != T_BOOL)                     // Whether traversed edges should be returned.
		return PROCEDURE_ERR;

	BFSContext *pdata = ctx->privateData;
	pdata->depleted = false;
	Node *source_node = args[0].ptrval;
	pdata->output[pdata->start_node_output_idx] = SI_Node(source_node);
	GrB_Index source_id = ENTITY_GET_ID(source_node);
	// TODO a level of 1 means "just the source", which I think may be unintuitive - consider adding 1 to make it "just neighbors".
	int64_t max_level = args[1].longval;
	const char *reltype = SIValue_IsNull(args[2]) ? NULL : args[2].stringval;
	bool collect_edges = args[3].longval;

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

	GrB_Vector V;  // Vector of results
	GrB_Vector PI = NULL; // Vector backtracking results to their parents.
	GrB_Vector *PI_ptr = &PI;
	if(!collect_edges) {
		PI_ptr = GrB_NULL;
		pdata->edges_output_idx = -1;
	}
	assert(LAGraph_bfs_both(&V, PI_ptr, R, TR, source_id, max_level, false) == GrB_SUCCESS);
	/* Remove all values with a level less than 2.
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
	assert(ctx->privateData);

	BFSContext *pdata = (BFSContext *)ctx->privateData;

	// Return NULL if this source has been mapped or there are no connected nodes.
	if(pdata->depleted || pdata->n == 0) return NULL;

	if(pdata->nodes_output_idx >= 0) {
		pdata->output[pdata->nodes_output_idx] = SI_Array(pdata->n);
	}

	if(pdata->edges_output_idx >= 0) {
		pdata->output[pdata->edges_output_idx] = SI_Array(pdata->n);
	}

	for(int i = 0; i < pdata->n; i ++) {
		NodeID node_id = pdata->node_ids[i];

		if(pdata->nodes_output_idx >= 0) {
			// Get the reached node.
			Node n = GE_NEW_NODE();
			Graph_GetNode(pdata->g, node_id, &n);
			SIArray_Append(&pdata->output[pdata->nodes_output_idx], SI_Node(&n));
		}

		if(pdata->edges_output_idx >= 0) {
			GrB_Index parent_id;
			assert(GrB_Vector_extractElement(&parent_id, pdata->parents, node_id) == GrB_SUCCESS);
			EdgeID id = Graph_GetSingleEdgeConnectingNodes(pdata->g, parent_id - 1, node_id, pdata->reltype_id);
			Edge e = GE_NEW_EDGE(parent_id - 1, node_id);
			Graph_GetEdge(pdata->g, id, &e);
			SIArray_Append(&pdata->output[pdata->edges_output_idx], SI_Edge(&e));
		}
	}

	pdata->depleted = true;

	return pdata->output;
}

static ProcedureResult Proc_BFS_Free(ProcedureCtx *ctx) {
	// Clean up.
	if(ctx->privateData) {
		BFSContext *pdata = ctx->privateData;
		if(pdata->output) array_free(pdata->output);
		if(pdata->node_ids) rm_free(pdata->node_ids);
		if(pdata->parents) GrB_Vector_free(&pdata->parents);
		rm_free(ctx->privateData);
	}

	return PROCEDURE_OK;
}

static BFSContext *_Proc_BFS_BuildContext(const char **yields, ProcedureOutput ***outputs) {
	// Setup context.
	BFSContext *pdata = rm_calloc(1, sizeof(BFSContext));
	pdata->g = QueryCtx_GetGraph();
	pdata->n = 0;
	pdata->reltype_id = GRAPH_NO_RELATION;

	pdata->nodes_output_idx = -1;
	pdata->edges_output_idx = -1;

	uint yield_count = array_len(yields);
	pdata->output = array_new(SIValue, yield_count * 2);
	for(uint i = 0; i < yield_count; i++) {
		ProcedureOutput *output = rm_malloc(sizeof(ProcedureOutput));
		// TODO Improve output logic to remove redundancy of internal and external outputs.
		if(!strcasecmp(yields[i], "start_node")) {
			// Internal
			pdata->output = array_append(pdata->output, SI_ConstStringVal("start_node"));
			pdata->start_node_output_idx = array_len(pdata->output);
			pdata->output = array_append(pdata->output, SI_Node(NULL)); // Place holder.

			// External
			output->name = "start_node";
			output->type = T_NODE;
		} else if(!strcasecmp(yields[i], "nodes")) {
			// Internal
			pdata->output = array_append(pdata->output, SI_ConstStringVal("nodes"));
			pdata->nodes_output_idx = array_len(pdata->output);
			pdata->output = array_append(pdata->output, SI_NullVal()); // Place holder.

			// External
			output->name = "nodes";
			output->type = T_ARRAY;
		} else if(!strcasecmp(yields[i], "edges")) {
			pdata->output = array_append(pdata->output, SI_ConstStringVal("edges"));
			pdata->edges_output_idx = array_len(pdata->output);
			pdata->output = array_append(pdata->output, SI_NullVal()); // Place holder.

			output->name = "edges";
			output->type = T_ARRAY;
		} else {
			assert(false);
		}
		*outputs = array_append(*outputs, output);
	}
	return pdata;
}

ProcedureCtx *Proc_BFS_Ctx(AR_ExpNode **args, const char **yields) {
	bool yield_edges = true;
	if(args) {
		SIValue yield_edges_val = AR_EXP_Evaluate(args[3], NULL);
		yield_edges = yield_edges_val.longval;
	}
	bool default_yields = (yields == NULL);
	if(default_yields) {
		int yield_count = 2;
		if(yield_edges) yield_count ++;
		yields = array_new(const char *, yield_count);
		yields = array_append(yields, "start_node");
		yields = array_append(yields, "nodes");
		if(yield_edges) yields = array_append(yields, "edges");
	}
	ProcedureOutput **outputs = array_new(ProcedureOutput *, array_len(yields));
	void *privdata = _Proc_BFS_BuildContext(yields, &outputs);
	if(default_yields) array_free(yields);

	ProcedureCtx *ctx = ProcCtxNew("algo.BFS",
								   4,
								   outputs,
								   Proc_BFS_Step,
								   Proc_BFS_Invoke,
								   Proc_BFS_Free,
								   privdata,
								   true);
	return ctx;
}

