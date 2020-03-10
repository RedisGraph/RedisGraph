/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "proc_shortest_path.h"
#include "proc_ctx.h"
#include "../query_ctx.h"
#include "../datatypes/array.h"
#include "../datatypes/path/path.h"
#include "../graph/entities/node.h"
#include "../algorithms/LAGraph_BF_full.h"

// CALL algo.shortestPath(startNode, endNode, weightProperty, relationshipQuery, defaultValue)

typedef struct {
	bool pathProduced;              // Indicates if a path was produced.
	Node *startNode;                // Start node to construct path from.
	Node *endNode;                  // End node, path ends with this node.
	const char *weightProperty;     // Attribute holding edge weight.
	const char *relationship;       // Type of relationship to traverse.
	double defaultValue;            // Use default value for edges missing weight attribute.
	GrB_Matrix M;                   // Weights matrix.
	SIValue *output;                // Output ["Path", path, "Cost", cost].
} ShortestPath_Ctx;

/* Construct NXN matrix M
 * where M's main diagonal M[i,i] = 0
 * for every edge of type `relation` connecting node i to node j
 * W[i,j] = weight_attr of edge. */
static GrB_Matrix _BuildWeightMatrix(const char *relation, const char *weight_attr,
									 double default_weight) {

	assert(relation && weight_attr);
	GrB_Matrix M = GrB_NULL;

	GraphContext *gc = QueryCtx_GetGraphCtx();
	Schema *s = GraphContext_GetSchema(gc, relation, SCHEMA_EDGE);
	if(!s) return GrB_NULL; // Relationship type doesn't exists.

	Attribute_ID attr_id = GraphContext_GetAttributeID(gc, weight_attr);
	if(attr_id == ATTRIBUTE_NOTFOUND) return GrB_NULL;  // Attribute doesn't exists.

	Graph *g = QueryCtx_GetGraph();
	GrB_Index n = Graph_RequiredMatrixDim(g);
	GrB_Matrix R = Graph_GetRelationMatrix(g, s->id);

	// TODO: enable FP64 in GB_control.h
	GrB_Info info = GrB_Matrix_new(&M, GrB_FP64, n, n);
	if(info != GrB_SUCCESS) return GrB_NULL;

	// Extract relations out of matrix.
	GrB_Index nz;
	GrB_Matrix_nvals(&nz, R);
	GrB_Index *I = rm_malloc(sizeof(GrB_Index) * nz);
	GrB_Index *J = rm_malloc(sizeof(GrB_Index) * nz);
	uint64_t  *X = rm_malloc(sizeof(uint64_t) * nz);
	info = GrB_Matrix_extractTuples_UINT64(I, J, X, &nz, R);

	// Process each relation.
	for(GrB_Index i = 0; i < nz; i++) {
		Edge e;
		EdgeID id = X[i];
		GrB_Index row = I[i];
		GrB_Index col = J[i];
		double weight = default_weight;

		if(SINGLE_EDGE(id)) {
			id = SINGLE_EDGE_ID(id);
			Graph_GetEdge(g, id, &e);
			SIValue *v = GraphEntity_GetProperty((GraphEntity *)&e, attr_id);
			SIValue_ToDouble(v, &weight);   // Doesn't modifies `weight` if `v` isn't numeric.
			GrB_Matrix_setElement_FP64(M, weight, row, col);
		} else {
			assert("TODO: handle multiple edges, pick min." && false);
		}
	}

	rm_free(I);
	rm_free(J);
	rm_free(X);
	return M;
}

/* Constructs a path object out of LAGraph BF output.
 * p - parent array
 * h - hops array
 * s_id - source node ID
 * t_id - destination node ID
 * Returns path object describing the shortest path from s to t */
static void _ConstructPath
(
	SIValue *path_output,   // Path object describing the shortest.
	SIValue *cost_output,   // Cost array specifying the cost to each node.
	GrB_Vector p,           // Parent tree.
	GrB_Vector h,           // Hops to node i.
	GrB_Vector d,           // Distance to node i.
	const ShortestPath_Ctx *ctx
) {

	Node n;
	GrB_Info info;
	Node *t = ctx->endNode;
	Node *s = ctx->startNode;
	NodeID s_id = ENTITY_GET_ID(s);
	NodeID t_id = ENTITY_GET_ID(t);
	Graph *g = QueryCtx_GetGraph();
	Edge *edges = array_new(Edge, 32);
	const char *relation = ctx->relationship;

	/* Determine path length M
	 * h[t_id] = number of hops on the shortest path from `s` to `t`. */
	uint64_t hops;
	info = GrB_Vector_extractElement_UINT64(&hops, h, t_id);
	assert(info == GrB_SUCCESS);

	// Construct path and cost array.
	SIValue cost = SIArray_New(hops);
	Path *path = Path_New(hops);
	Path_AppendNode(path, *t);

	double c;
	info = GrB_Vector_extractElement_FP64(&c, d, t_id);
	SIArray_Append(&cost, SI_DoubleVal(c));

	// Determine traversed edge type.
	GraphContext *gc = QueryCtx_GetGraphCtx();
	Schema *schema = GraphContext_GetSchema(gc, relation, SCHEMA_EDGE);
	int r_id = schema->id;

	while(hops) {
		/* p[t_id] contains the source node leading to parent
		 * on the shortest path from `s` to `t`. */
		info = GrB_Vector_extractElement_UINT64(&s_id, p, t_id);
		assert(info == GrB_SUCCESS);
		s_id--; // as p is 1 index based.

		// Get edge connecting src to dest.
		Graph_GetEdgesConnectingNodes(g, s_id, t_id, r_id, &edges);
		if(array_len(edges) > 1) {
			// TODO: handle multiple edges.
			assert("handle multiple edges" && false);
		}

		Path_AppendEdge(path, edges[0]);
		array_clear(edges);

		Graph_GetNode(g, s_id, &n);
		Path_AppendNode(path, n);

		info = GrB_Vector_extractElement_FP64(&c, d, s_id);
		SIArray_Append(&cost, SI_DoubleVal(c));

		// Update
		t_id = s_id;
		hops--;
	}

	array_free(edges);
	// Reverse path such that source node is at the beginning.
	Path_Reverse(path);
	SIArray_Reverse(cost);

	*path_output = SI_Path(path);
	*cost_output = cost;
}

ProcedureResult Proc_ShortestPathInvoke(ProcedureCtx *ctx, const SIValue *args) {
	if(array_len((SIValue *)args) != 5) return PROCEDURE_ERR;

	ctx->privateData = NULL;

	Node *startNode = (Node *)(args[0].ptrval);
	Node *endNode = (Node *)(args[1].ptrval);
	const char *weightProperty = args[2].stringval;
	const char *relationship = args[3].stringval;
	double defaultValue = args[4].doubleval;

	GrB_Matrix M = _BuildWeightMatrix(relationship, weightProperty, defaultValue);
	if(M == GrB_NULL) return PROCEDURE_ERR;

	ShortestPath_Ctx *pdata = rm_malloc(sizeof(ShortestPath_Ctx));
	pdata->M = M;
	pdata->endNode = endNode;
	pdata->startNode = startNode;
	pdata->relationship = relationship;
	pdata->defaultValue = defaultValue;
	pdata->weightProperty = weightProperty;
	pdata->pathProduced = false;

	pdata->output = array_new(SIValue, 4);
	pdata->output = array_append(pdata->output, SI_ConstStringVal("path"));
	pdata->output = array_append(pdata->output, SI_NullVal()); // Place holder.
	pdata->output = array_append(pdata->output, SI_ConstStringVal("cost"));
	pdata->output = array_append(pdata->output, SI_NullVal()); // Place holder.

	ctx->privateData = pdata;
	return PROCEDURE_OK;
}

SIValue *Proc_ShortestPathStep(ProcedureCtx *ctx) {
	assert(ctx->privateData);

	SIValue *res = NULL;
	ShortestPath_Ctx *pdata = (ShortestPath_Ctx *)ctx->privateData;
	Node *s = pdata->startNode;
	Node *t = pdata->endNode;
	NodeID s_id = ENTITY_GET_ID(s);
	NodeID t_id = ENTITY_GET_ID(t);
	GrB_Vector d = GrB_NULL;    // Pointer to the vector of distance
	GrB_Vector pi = GrB_NULL;   // Pointer to the vector of parent
	GrB_Vector h = GrB_NULL;    // Pointer to the vector of hops

	if(pdata->pathProduced) goto cleanup;
	// Mark this call to Step, such that additional calls will return NULL.
	pdata->pathProduced = true;

	GrB_Info info = LAGraph_BF_full(&d, &pi, &h, pdata->M, s_id);

	if(info != GrB_SUCCESS) {
		/* Failed to run Bellman-Ford single source shortest paths.
		 * return NULL. */
		goto cleanup;
	}

	// Determine number of hops on shortest path.
	uint64_t hops;
	GrB_Vector_extractElement_UINT64(&hops, h, t_id);
	if(hops == 0) {
		// No path from s to t.
		goto cleanup;
	}

	// Set result.
	res = pdata->output;

	// s and t are connected, construct PATH object.
	SIValue path;
	SIValue cost;
	_ConstructPath(&path, &cost, pi, h, d, pdata);
	pdata->output[1] = path;
	pdata->output[3] = cost;

cleanup:
	if(d) GrB_free(&d);
	if(pi) GrB_free(&pi);
	if(h) GrB_free(&h);

	// Mark ourselves as depleted.
	return res;
}

ProcedureResult Proc_ShortestPathFree(ProcedureCtx *ctx) {
	// Clean up.
	if(ctx->privateData) {
		ShortestPath_Ctx *pdata = ctx->privateData;
		GrB_free(&pdata->M);
		array_free(pdata->output);
		rm_free(ctx->privateData);
	}

	return PROCEDURE_OK;
}

ProcedureCtx *Proc_ShortestPathCtx() {
	void *privateData = NULL;
	ProcedureOutput **outputs = array_new(ProcedureOutput *, 2);

	ProcedureOutput *output_path = rm_malloc(sizeof(ProcedureOutput));
	output_path->name = "path";
	output_path->type = T_PATH;

	ProcedureOutput *output_cost = rm_malloc(sizeof(ProcedureOutput));
	output_cost->name = "cost";
	output_cost->type = T_ARRAY;

	outputs = array_append(outputs, output_path);
	outputs = array_append(outputs, output_cost);

	unsigned int ninputs = 5;    // Start node, end node, weight property, edge type, default weight.
	ProcedureCtx *ctx = ProcCtxNew("algo.shortestPath",
								   ninputs,
								   outputs,
								   Proc_ShortestPathStep,
								   Proc_ShortestPathInvoke,
								   Proc_ShortestPathFree,
								   privateData,
								   true);
	return ctx;
}
