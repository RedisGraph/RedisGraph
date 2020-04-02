/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "path_funcs.h"
#include "../func_desc.h"
#include "../../datatypes/path/sipath_builder.h"
#include "../../ast/ast.h"
#include "../../util/rmalloc.h"
#include "../../util/arr.h"
#include "assert.h"

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
	assert(argc == (nelements + 1));

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
	func_desc = AR_FuncDescNew("topath", AR_TOPATH, 1, VAR_ARG_LEN, types, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, T_NULL | T_PATH);
	func_desc = AR_FuncDescNew("nodes", AR_PATH_NODES, 1, 1, types, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, T_NULL | T_PATH);
	func_desc = AR_FuncDescNew("relationships", AR_PATH_RELATIONSHIPS, 1, 1, types, false);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 1);
	types = array_append(types, T_NULL | T_PATH);
	func_desc = AR_FuncDescNew("length", AR_PATH_LENGTH, 1, 1, types, false);
	AR_RegFunc(func_desc);
}

