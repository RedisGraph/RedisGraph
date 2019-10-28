/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "path_funcs.h"
#include "../func_desc.h"
#include "../../datatypes/sipath.h"
#include "../../ast/ast.h"
#include "../../util/rmalloc.h"
#include "../../util/arr.h"
#include "assert.h"

/**
 * @brief  Appends node into a path object during path building.
 * @param  path: A pointer to path objects.
 * @param  n: Node.
 * @retval None
 */
static void _PathBuilder_AppendNode(Path *path, Node *node) {
	Path_AppendNode(path, *node);
}

/**
 * @brief  Reverse the direction of an edge.
 * @note   A new copy of the edge is created, in order not to invalidate any data in the original edge
 *         in case it required elsewhere.
 * @param  *e: Pointer to the original edge.
 * @retval New edge with inverse direction.
 */
static Edge *_Edge_ReverseDirection(Edge *e) {
	Edge *clone = Edge_Clone(e);
	clone->srcNodeID = e->destNodeID;
	clone->destNodeID = e->srcNodeID;
	return clone;
}

/**
 * @brief  Appends an edge to path object during path build.
 * @note   The edge should be added after its source node has been inserted to its right position in the path.
 *         Edges insertion is done by interliving nodes and edges.
 * @param  path: Path.
 * @param  e: Edge.
 * @param  RTLEdge: Indicates the if the edge is incoming or outgoing edge (RTL in query).
 * @retval None
 */
static void _PathBuilder_AppendEdge(Path *path, Edge *edge, bool RTLEdge) {
	assert(Path_NodeCount(*path) > 0);
	// The edge should connect nodes[edge_count] to nodes[edge_count+1]
	uint edge_count = Path_EdgeCount(*path);
	Node n = (*path).nodes[edge_count];
	EntityID nId = ENTITY_GET_ID(&n);
	// Validate source node is in the right place.
	assert(nId == edge->srcNodeID || nId == edge->destNodeID);
	/* Reverse direction if needed. A direction change is needed if the last node in the path, reading
	 * RTL is the source node in the edge, and the edge direction in the query is LTR.
	 * path =[(a)]
	 * e = (a)->(b)
	 * Query: MATCH p=(a)<-[]-(b)
	 * e direction needs to be change. */

	if(RTLEdge && nId == edge->srcNodeID) edge = _Edge_ReverseDirection(edge);
	Path_AppendEdge(path, *edge);
}

// The function will append new path to an existing path. If the new path is empty, the function returns true.
static bool _PathBuilder_AppendPath(Path *path, Path *new_path, bool RTLEdge) {
	uint path_node_count = Path_NodeCount(*path);
	assert(path_node_count > 0);
	// No need to append empty paths.
	uint new_path_node_count = Path_NodeCount(*new_path);

	if(new_path_node_count <= 1) return false;

	Node *last_LTR_node = &path->nodes[path_node_count - 1];
	EntityID last_LTR_node_id = ENTITY_GET_ID(last_LTR_node);
	Node *new_path_node_0 = &new_path->nodes[0];
	EntityID new_path_node_0_id = ENTITY_GET_ID(new_path_node_0);
	Node *new_path_last_node = &new_path->nodes[new_path_node_count - 1];
	EntityID new_path_last_node_id = ENTITY_GET_ID(new_path_last_node);
	// Validate current last LTR node is in either edges of the path.
	assert(last_LTR_node_id == new_path_node_0_id || last_LTR_node_id == new_path_last_node_id);
	int new_path_edge_count = Path_EdgeCount(*new_path);

	// Check if path needs to be rverse inserated or not.
	if(last_LTR_node_id == new_path_last_node_id) Path_Reverse(*new_path);

	for(uint i = 0; i < new_path_edge_count - 1; i++) {
		// Insert only nodes which are not the last and the first, since they will be added by append node specifically.
		_PathBuilder_AppendNode(path, &new_path->nodes[i + 1]);
		_PathBuilder_AppendEdge(path, &new_path->edges[i], RTLEdge);
	}
	_PathBuilder_AppendEdge(path, &new_path->edges[new_path_edge_count - 1], RTLEdge);

	return true;
}

/* Creates a path from a given sequence of graph entities.
 * The first argument is the ast represents the path.
 * Arguments 2...n are the sequence of graph entities combines the path.
 * The sequence is always in odd length and defined as:
 * Odd indices members are always representing the value of a single node.
 * Even indices members are either representing the value of a single edge,
 * or an sipath, in case of variable length traversal. */
SIValue AR_TOPATH(SIValue *argv, int argc) {
	const cypher_astnode_t *ast_path = argv[0].ptrval;
	uint nelements = cypher_ast_pattern_path_nelements(ast_path);
	assert(argc == (nelements + 1));
	Path path = Path_New(nelements);
	for(uint i = 0; i < nelements; i++) {
		SIValue element = argv[i + 1];
		if(i % 2 == 0) {
			// Nodes are in even position.
			_PathBuilder_AppendNode(&path, element.ptrval);
		} else {
			// Edges and paths are in odd positions.
			const cypher_astnode_t *ast_edge = cypher_ast_pattern_path_get_element(ast_path, i);
			bool RTL_edge = cypher_ast_rel_pattern_get_direction(ast_edge) == CYPHER_REL_INBOUND;
			// Element type can be either edge, or path.
			if(element.type == T_EDGE) _PathBuilder_AppendEdge(&path, element.ptrval, RTL_edge);
			// If element is not an edge, it is a path. If it is an empty path, end the build.
			else if(!_PathBuilder_AppendPath(&path, element.ptrval, RTL_edge)) break;
		}
	}
	return SI_Path(&path);
}

SIValue AR_PATH_NODES(SIValue *argv, int argc) {
	return SIPath_Nodes(argv[0]);
}

SIValue AR_PATH_RELATIONSHIPS(SIValue *argv, int argc) {
	return SIPath_Relationships(argv[0]);
}

SIValue AR_PATH_LENGTH(SIValue *argv, int argc) {
	return SI_LongVal(SIPath_Length(argv[0]));
}

void Register_PathFuncs() {
	SIType *types;
	AR_FuncDesc *func_desc;

	types = array_new(SIType, 2);
	types = array_append(types, T_PTR);
	types = array_append(types, T_NODE | T_EDGE | T_PATH);
	func_desc = AR_FuncDescNew("topath", AR_TOPATH, VAR_ARG_LEN, types, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, T_PATH);
	func_desc = AR_FuncDescNew("nodes", AR_PATH_NODES, 1, types, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, T_PATH);
	func_desc = AR_FuncDescNew("relationships", AR_PATH_RELATIONSHIPS, 1, types, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, T_PATH);
	func_desc = AR_FuncDescNew("length", AR_PATH_LENGTH, 1, types, false);
	AR_RegFunc(func_desc);
}
