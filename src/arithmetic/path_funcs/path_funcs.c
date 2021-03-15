/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "path_funcs.h"
#include "../func_desc.h"
#include "../../config.h"
#include "../../ast/ast.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"
#include "../../util/rmalloc.h"
#include "../../datatypes/path/sipath_builder.h"
#include "../../algorithms/LAGraph_bfs_pushpull.h"

/* Creates a path from a given sequence of graph entities.
 * The first argument is the ast node represents the path.
 * Arguments 2...n are the sequence of graph entities combines the path.
 * The sequence is always in odd length and defined as:
 * Odd indices members are always representing the value of a single node.
 * Even indices members are either representing the value of a single edge,
 * or an sipath, in case of variable length traversal. */
SIValue AR_TOPATH(SIValue *argv, int argc) {
	const cypher_astnode_t *ast_path = argv[0].ptrval;
	uint nelements = cypher_ast_pattern_path_nelements(ast_path);
	ASSERT(argc == (nelements + 1));

	SIValue path = SIPathBuilder_New(nelements);
	for(uint i = 0; i < nelements; i++) {
		SIValue element = argv[i + 1];
		if(SI_TYPE(element) == T_NULL) {
			/* If any element of the path does not exist, the entire path is invalid.
			 * Free it and return a null value. */
			SIValue_Free(path);
			return SI_NullVal();
		}

		if(i % 2 == 0) {
			// Nodes are in even position.
			SIPathBuilder_AppendNode(path, element);
		} else {
			// Edges and paths are in odd positions.
			const cypher_astnode_t *ast_rel_pattern = cypher_ast_pattern_path_get_element(ast_path, i);
			bool RTL_pattern = cypher_ast_rel_pattern_get_direction(ast_rel_pattern) == CYPHER_REL_INBOUND;
			// Element type can be either edge, or path.
			if(SI_TYPE(element) == T_EDGE) SIPathBuilder_AppendEdge(path, element, RTL_pattern);
			// If element is not an edge, it is a path.
			else {
				/* Path with 0 edges should not be appended. Their source and destination nodes are the same,
				 * and the source node already appended.
				 * The build should continue to the next edge/path value. Consider the following query:
				 * "MATCH p=(a:L1)-[*0..]->(b:L1)-[]->(c:L2)" for the graph in the form of (:L1)-[]->(:L2). The path build should
				 * return a path with with the relevant entities.
				 */
				if(SIPath_Length(element) == 0) {
					i++;
					continue;
				}
				SIPathBuilder_AppendPath(path, element, RTL_pattern);
			}
		}
	}
	return path;
}

// Routine for freeing a shortest path function's private data.
void ShortestPath_Free(void *ctx_ptr) {
	ShortestPathCtx *ctx = ctx_ptr;
	if(ctx->reltypes) array_free(ctx->reltypes);
	if(ctx->free_matrices) {
		GrB_free(&ctx->R);
		GrB_free(&ctx->TR);
	}
	rm_free(ctx);
}

// Routine for cloning a shortest path function's private data.
void *ShortestPath_Clone(void *orig) {
	ShortestPathCtx *ctx = orig;
	// Allocate space for the clone
	ShortestPathCtx *ctx_clone = rm_malloc(sizeof(ShortestPathCtx));
	ctx_clone->minHops = ctx->minHops;
	ctx_clone->maxHops = ctx->maxHops;
	if(ctx->reltypes) array_clone(ctx_clone->reltypes, ctx->reltypes);
	else ctx_clone->reltypes = NULL;
	// Do not clone matrix data
	ctx_clone->R = GrB_NULL;
	ctx_clone->TR = GrB_NULL;
	ctx_clone->free_matrices = false;

	return ctx_clone;
}

SIValue AR_SHORTEST_PATH(SIValue *argv, int argc) {
	if(SI_TYPE(argv[0]) == T_NULL) return SI_NullVal();
	if(SI_TYPE(argv[1]) == T_NULL) return SI_NullVal();

	Node             *srcNode   =  argv[0].ptrval;
	Node             *destNode  =  argv[1].ptrval;
	ShortestPathCtx  *ctx       =  argv[2].ptrval;
	GrB_Index        src_id     =  ENTITY_GET_ID(srcNode);
	GrB_Index        dest_id    =  ENTITY_GET_ID(destNode);

	GrB_Info res;
	UNUSED(res);
	Edge *edges = NULL;
	GrB_Vector V = GrB_NULL;  // vector of results
	GrB_Vector PI = GrB_NULL; // vector backtracking results to their parents
	GraphContext *gc = QueryCtx_GetGraphCtx();
	uint reltype_count = (ctx->reltypes) ? array_len(ctx->reltypes) : 0;

	/* The BFS algorithm uses a level of 1 to indicate the source node.
	 * If this value is not zero (unlimited), increment it by 1
	 * to make level 1 indicate the source's direct neighbors. */
	int64_t max_level = (ctx->maxHops == EDGE_LENGTH_INF) ? 0 : ctx->maxHops + 1;

	if(ctx->R == GrB_NULL) {
		// First invocation, initialize the traversed matrices.
		bool maintain_transposes;
		Config_Option_get(Config_MAINTAIN_TRANSPOSE, &maintain_transposes);
		// Get edge matrix and transpose matrix, if available.
		if(reltype_count == 0) {
			ctx->R = Graph_GetAdjacencyMatrix(gc->g);
			ctx->TR = Graph_GetTransposedAdjacencyMatrix(gc->g);
		} else if(reltype_count == 1) {
			ctx->R = Graph_GetRelationMatrix(gc->g, ctx->reltypes[0]);
			if(maintain_transposes) ctx->TR = Graph_GetTransposedRelationMatrix(gc->g, ctx->reltypes[0]);
			else ctx->TR = GrB_NULL;
		} else {
			// We have multiple edge types, combine them into a boolean matrix.
			ctx->free_matrices = true;
			GrB_Index dims = Graph_RequiredMatrixDim(gc->g);
			GrB_Matrix_new(&ctx->R, GrB_BOOL, dims, dims);
			if(maintain_transposes) GrB_Matrix_new(&ctx->TR, GrB_BOOL, dims, dims);
			for(uint i = 0; i < reltype_count; i ++) {
				GrB_Matrix adj = Graph_GetRelationMatrix(gc->g, ctx->reltypes[i]);
				res = GrB_eWiseAdd(ctx->R, GrB_NULL, GrB_NULL, GxB_ANY_PAIR_BOOL, ctx->R, adj, GrB_NULL);
				ASSERT(res == GrB_SUCCESS);
				if(maintain_transposes) {
					GrB_Matrix adj = Graph_GetTransposedRelationMatrix(gc->g, ctx->reltypes[i]);
					res = GrB_eWiseAdd(ctx->TR, GrB_NULL, GrB_NULL, GxB_ANY_PAIR_BOOL, ctx->TR, adj, GrB_NULL);
					ASSERT(res == GrB_SUCCESS);
				}
			}
		}
	}

	// Invoke the BFS algorithm
	res = LAGraph_bfs_pushpull_to_dest(&V, &PI, ctx->R, ctx->TR, src_id,
			dest_id, max_level, true);
	ASSERT(res == GrB_SUCCESS);

	SIValue p = SI_NullVal();

	// The length of the path is equal to the level of the destination node
	GrB_Index path_len;
	res = GrB_Vector_extractElement(&path_len, V, dest_id);
	path_len -= 1; // Convert node count to edge count

	if(res == GrB_NO_VALUE) goto cleanup; // no path found

	// Only emit a path with no edges if minHops is 0
	if(path_len == 0 && ctx->minHops != 0) goto cleanup;

	// Build path in reverse, starting by appending the destination node
	p = SIPathBuilder_New(path_len);
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
		if(reltype_count == 0) {
			Graph_GetEdgesConnectingNodes(gc->g, parent_id, id, GRAPH_NO_RELATION, &edges);
		} else {
			for(uint j = 0; j < reltype_count; j ++) {
				Graph_GetEdgesConnectingNodes(gc->g, parent_id, id, ctx->reltypes[j], &edges);
				if(array_len(edges) > 0) break;
			}
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

cleanup:
	if(V) GrB_free(&V);
	if(PI) GrB_free(&PI);
	if(edges) array_free(edges);

	return p;
}

SIValue AR_PATH_NODES(SIValue *argv, int argc) {
	if(SI_TYPE(argv[0]) == T_NULL) return SI_NullVal();
	return SIPath_Nodes(argv[0]);
}

SIValue AR_PATH_RELATIONSHIPS(SIValue *argv, int argc) {
	if(SI_TYPE(argv[0]) == T_NULL) return SI_NullVal();
	return SIPath_Relationships(argv[0]);
}

SIValue AR_PATH_LENGTH(SIValue *argv, int argc) {
	if(SI_TYPE(argv[0]) == T_NULL) return SI_NullVal();
	return SI_LongVal(SIPath_Length(argv[0]));
}

void Register_PathFuncs() {
	SIType *types;
	AR_FuncDesc *func_desc;

	types = array_new(SIType, 2);
	types = array_append(types, T_PTR);
	types = array_append(types, T_NULL | T_NODE | T_EDGE | T_PATH);
	func_desc = AR_FuncDescNew("topath", AR_TOPATH, 1, VAR_ARG_LEN, types, false, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 3);
	types = array_append(types, T_NULL | T_NODE);
	types = array_append(types, T_NULL | T_NODE);
	types = array_append(types, T_PTR);
	func_desc = AR_FuncDescNew("shortestpath", AR_SHORTEST_PATH, 3, 3, types, false, false);
	AR_SetPrivateDataRoutines(func_desc, ShortestPath_Free, ShortestPath_Clone);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, T_NULL | T_PATH);
	func_desc = AR_FuncDescNew("nodes", AR_PATH_NODES, 1, 1, types, false, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, T_NULL | T_PATH);
	func_desc = AR_FuncDescNew("relationships", AR_PATH_RELATIONSHIPS, 1, 1, types, false, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, T_NULL | T_PATH);
	func_desc = AR_FuncDescNew("length", AR_PATH_LENGTH, 1, 1, types, false, false);
	AR_RegFunc(func_desc);
}

