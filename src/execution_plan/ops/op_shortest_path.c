/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_shortest_path.h"
#include "shared/print_functions.h"
#include "../../config.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"
#include "../../graph/graphcontext.h"
#include "../../datatypes/path/sipath_builder.h"
#include "../../algorithms/LAGraph_bfs_pushpull.h"
#include "../../arithmetic/arithmetic_expression.h"

/* Forward declarations. */
static Record ShortestPathConsume(OpBase *opBase);
static OpResult ShortestPathReset(OpBase *opBase);
static OpBase *ShortestPathClone(const ExecutionPlan *plan, const OpBase *opBase);
static void ShortestPathFree(OpBase *opBase);

static void _setupTraversedRelations(OpShortestPath *op) {
	QGEdge *e = QueryGraph_GetEdgeByAlias(op->op.plan->query_graph, AlgebraicExpression_Edge(op->ae));
	ASSERT(e->minHops <= e->maxHops);
	op->minHops = e->minHops;
	op->maxHops = e->maxHops;

	uint reltype_count = array_len(e->reltypeIDs);
	if(reltype_count == 0) {
		op->edgeRelationCount = 1;
		op->edgeRelationTypes = array_new(int, 1);
		op->edgeRelationTypes = array_append(op->edgeRelationTypes, GRAPH_NO_RELATION);
	} else {
		GraphContext *gc = QueryCtx_GetGraphCtx();
		op->edgeRelationCount = 0;
		op->edgeRelationTypes = array_new(int, reltype_count);

		for(int i = 0; i < reltype_count; i++) {
			int rel_id = e->reltypeIDs[i];
			if(rel_id != GRAPH_UNKNOWN_RELATION) {
				op->edgeRelationTypes = array_append(op->edgeRelationTypes, rel_id);
			} else {
				const char *rel_type = e->reltypes[i];
				Schema *s = GraphContext_GetSchema(gc, rel_type, SCHEMA_EDGE);
				if(s) op->edgeRelationTypes = array_append(op->edgeRelationTypes, s->id);
			}
		}

		op->edgeRelationCount = array_len(op->edgeRelationTypes);
	}
}

// Set the traversal direction to match the traversed edge and AlgebraicExpression form.
static inline void _setTraverseDirection(OpShortestPath *op, const QGEdge *e) {
	if(e->bidirectional) {
		op->traverseDir = GRAPH_EDGE_DIR_BOTH;
	} else {
		if(AlgebraicExpression_Transposed(op->ae)) {
			// traverse in the opposite direction, (dest)->(src) incoming edges
			op->traverseDir = GRAPH_EDGE_DIR_INCOMING;
		} else {
			op->traverseDir = GRAPH_EDGE_DIR_OUTGOING;
		}
	}
}

static inline int ShortestPathToString(const OpBase *ctx, char *buf, uint buf_len) {
	// TODO: tmp, improve TraversalToString
	AlgebraicExpression_Optimize(&((OpShortestPath *)ctx)->ae);
	return TraversalToString(ctx, buf, buf_len, ((const OpShortestPath *)ctx)->ae);
}

void ShortestPathOp_ExpandInto(OpShortestPath *op) {
	// Expand into doesn't performs any modifications.
	array_clear(op->op.modifies);
	op->expandInto = true;
	op->op.type = OPType_CONDITIONAL_VAR_LEN_TRAVERSE_EXPAND_INTO;
	op->op.name = "Shortest Path (Expand Into)";
}

OpBase *NewShortestPathOp(const ExecutionPlan *plan, Graph *g, AlgebraicExpression *ae,
						  TRAVERSE_MODE mode) {
	ASSERT(g != NULL);
	ASSERT(ae != NULL);

	OpShortestPath *op = rm_malloc(sizeof(OpShortestPath));
	op->g = g;
	op->ae = ae;
	op->mode = mode;
	op->expandInto = false;
	op->edgeRelationTypes = NULL;

	OpBase_Init((OpBase *)op, OPType_SHORTEST_PATH,
				"Shortest Path", NULL, ShortestPathConsume, ShortestPathReset,
				ShortestPathToString, ShortestPathClone, ShortestPathFree, false, plan);

	bool aware = OpBase_Aware((OpBase *)op, AlgebraicExpression_Source(ae), &op->srcNodeIdx);
	ASSERT(aware);
	op->destNodeIdx = OpBase_Modifies((OpBase *)op, AlgebraicExpression_Destination(ae));

	// populate edge value in record only if it is referenced
	AST *ast = QueryCtx_GetAST();
	QGEdge *e = QueryGraph_GetEdgeByAlias(plan->query_graph, AlgebraicExpression_Edge(op->ae));
	op->pathIdx = AST_AliasIsReferenced(ast, e->alias) ? OpBase_Modifies((OpBase *)op, e->alias) : -1;
	_setTraverseDirection(op, e); // TODO our algorithm can't do undirected

	return (OpBase *)op;
}

static Record ShortestPathConsume(OpBase *opBase) {
	OpShortestPath *op = (OpShortestPath *)opBase;
	OpBase *child = op->op.children[0];
	Edge *edges = NULL;
	GrB_Info res;
	UNUSED(res);
	GrB_Matrix R = GrB_NULL;  // Traversed relationship matrix
	GrB_Matrix TR = GrB_NULL; // Transpose of traversed relationship matrix
	GrB_Vector V = GrB_NULL;  // Vector of results
	GrB_Vector PI = GrB_NULL; // Vector backtracking results to their parents.
	bool free_matrices = false;

	Record r = OpBase_Consume(child);
	if(!r) return NULL;

	// Create edge relation type array on first call to consume.
	if(!op->edgeRelationTypes) {
		_setupTraversedRelations(op);
		/* Incase we don't have any relations to traverse and minimal traversal is at least one hop
		 * we can return quickly.
		 * Consider: MATCH (S)-[:L*]->(M) RETURN M
		 * where label L does not exists. */
		if(op->edgeRelationCount == 0 && op->minHops > 0) return NULL;
	}

	Node *srcNode = Record_GetNode(r, op->srcNodeIdx);
	if(srcNode == NULL) {
		/* The child Record may not contain the source node in scenarios like
		 * a failed OPTIONAL MATCH. In this case, delete the Record and try again. */
		OpBase_DeleteRecord(r);
		r = NULL;
		goto cleanup;
	}
	GrB_Index src_id = ENTITY_GET_ID(srcNode);

	Node *destNode = Record_GetNode(r, op->destNodeIdx);
	if(destNode == NULL) {
		OpBase_DeleteRecord(r);
		r = NULL;
		goto cleanup;
	}
	GrB_Index dest_id = ENTITY_GET_ID(destNode);

	/* The BFS algorithm uses a level of 1 to indicate the source node.
	 * If this value is not zero (unlimited), increment it by 1
	 * to make level 1 indicate the source's direct neighbors. */
	int64_t max_level = (op->maxHops == EDGE_LENGTH_INF) ? 0 : op->maxHops + 1;

	// Get edge matrix and transpose matrix, if available.
	GraphContext *gc = QueryCtx_GetGraphCtx();
	bool maintain_transpose;
	Config_Option_get(Config_MAINTAIN_TRANSPOSE, &maintain_transpose);
	if(op->edgeRelationCount == 1) {
		if(op->edgeRelationTypes[0] == GRAPH_NO_RELATION) {
			R = Graph_GetAdjacencyMatrix(gc->g);
			TR = Graph_GetTransposedAdjacencyMatrix(gc->g);
		} else {
			R = Graph_GetRelationMatrix(gc->g, op->edgeRelationTypes[0]);
			if(maintain_transpose) TR = Graph_GetTransposedRelationMatrix(gc->g, op->edgeRelationTypes[0]);
			else TR = GrB_NULL;
		}
	} else {
		// We have multiple edge types, combine them into a boolean matrix.
		free_matrices = true;
		GrB_Index dims = Graph_RequiredMatrixDim(gc->g);
		GrB_Matrix_new(&R, GrB_BOOL, dims, dims);
		if(maintain_transpose) GrB_Matrix_new(&TR, GrB_BOOL, dims, dims);
		for(uint i = 0; i < op->edgeRelationCount; i ++) {
			GrB_Matrix adj = Graph_GetRelationMatrix(gc->g, op->edgeRelationTypes[i]);
			res = GrB_eWiseAdd(R, GrB_NULL, GrB_NULL, GxB_ANY_PAIR_BOOL, R, adj, GrB_NULL);
			ASSERT(res == GrB_SUCCESS);
			if(maintain_transpose) {
				GrB_Matrix adj = Graph_GetTransposedRelationMatrix(gc->g, op->edgeRelationTypes[i]);
				res = GrB_eWiseAdd(TR, GrB_NULL, GrB_NULL, GxB_ANY_PAIR_BOOL, TR, adj, GrB_NULL);
				ASSERT(res == GrB_SUCCESS);
			}
		}
	}

	res = LAGraph_bfs_pushpull_to_dest(&V, &PI, R, TR, src_id, dest_id, max_level, true);
	ASSERT(res == GrB_SUCCESS);

	// GxB_print(V, GxB_COMPLETE);
	// GxB_print(PI, GxB_COMPLETE);

	GrB_Index path_len;
	res = GrB_Vector_extractElement(&path_len, V, dest_id) ;
	path_len -= 1; // Convert node count to edge count
	if(res == GrB_NO_VALUE) {
		OpBase_DeleteRecord(r);
		r = NULL;
		goto cleanup; // no path found
	}
	// Only emit a path with no edges if minHops is 0
	if(path_len == 0 && op->minHops != 0) {
		OpBase_DeleteRecord(r);
		r = NULL;
		goto cleanup;
	}

	// Return early if the query does not reference the path
	if(op->pathIdx < 0) goto cleanup;

	// Build path in reverse
	SIValue p = SIPathBuilder_New(path_len);
	SIPathBuilder_AppendNode(p, SI_Node(destNode));

	edges = array_new(Edge, 1);

	NodeID id = destNode->id;
	for(uint i = 0; i < path_len; i ++) {
		array_clear(edges);
		GrB_Index parent_id;
		// Find the parent of the reached node.
		GrB_Info res = GrB_Vector_extractElement(&parent_id, PI, id);
		ASSERT(res == GrB_SUCCESS);
		parent_id --; // Decrement the parent ID by 1 to correct 1-indexing.

		// Retrieve edges connecting the parent node to the current node.
		for(uint j = 0; j < op->edgeRelationCount; j ++) {
			Graph_GetEdgesConnectingNodes(gc->g, parent_id, id, op->edgeRelationTypes[j], &edges);
			if(array_len(edges) > 0) break;
		}
		ASSERT(array_len(edges) > 0);
		// Append the edge to the path
		SIPathBuilder_AppendEdge(p, SI_Edge(&edges[0]), false);

		// Append the reached node to the path.
		id = edges[0].srcNodeID;
		Node n = GE_NEW_NODE();
		Graph_GetNode(gc->g, id, &n);
		SIPathBuilder_AppendNode(p, SI_Node(&n));
	}

	// Reverse the path so it starts at the source
	Path_Reverse(p.ptrval);

	// Add new path to Record.
	Record_AddScalar(r, op->pathIdx, p);

cleanup:
	if(free_matrices) {
		GrB_free(&R);
		GrB_free(&TR);
	}
	if(V) GrB_free(&V);
	if(PI) GrB_free(&PI);
	if(edges) array_free(edges);
	return r;
}

static OpResult ShortestPathReset(OpBase *ctx) {
	OpShortestPath *op = (OpShortestPath *)ctx;
	return OP_OK;
}

static OpBase *ShortestPathClone(const ExecutionPlan *plan, const OpBase *opBase) {
	ASSERT(opBase->type == OPType_SHORTEST_PATH);
	OpShortestPath *op = (OpShortestPath *) opBase;
	OpBase *op_clone = NewShortestPathOp(plan, QueryCtx_GetGraph(),
										 AlgebraicExpression_Clone(op->ae), op->mode);
	return op_clone;
}

static void ShortestPathFree(OpBase *ctx) {
	OpShortestPath *op = (OpShortestPath *)ctx;

	if(op->edgeRelationTypes) {
		array_free(op->edgeRelationTypes);
		op->edgeRelationTypes = NULL;
	}

	if(op->ae) {
		AlgebraicExpression_Free(op->ae);
		op->ae = NULL;
	}
}

