/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "ast.h"
#include "../errors.h"
#include "../util/arr.h"

static cypher_astnode_t *_ExpandPath(const cypher_astnode_t *path,
									 const cypher_astnode_t *edge, uint idx, uint edge_len, uint path_len) {
	cypher_astnode_t *children[path_len + (edge_len * 2)];
	// clone all path elements preceding the edge to expand
	for(uint i = 0; i < idx; i ++) {
		children[i] =
			cypher_ast_clone(cypher_ast_pattern_path_get_element(path, i));
	}

	// retrieve edge data from edge to expand
	enum cypher_rel_direction dir = cypher_ast_rel_pattern_get_direction(edge);
	uint reltype_count = cypher_ast_rel_pattern_nreltypes(edge);
	struct cypher_input_range range = cypher_astnode_range(edge);
	cypher_astnode_t *reltypes[reltype_count];

	uint new_idx = idx;
	for(uint i = 0; i < edge_len; i ++) {
		// clone the reltypes of the edge
		for(uint j = 0; j < reltype_count; j ++) {
			cypher_astnode_t *reltype =
				cypher_ast_clone(cypher_ast_rel_pattern_get_reltype(edge, j));
			reltypes[j] = reltype;
		}
		// create new edge
		children[new_idx + i] = cypher_ast_rel_pattern(dir,
													   NULL,
													   ((reltype_count == 0) ? NULL : reltypes),
													   reltype_count,
													   NULL,
													   NULL,
													   ((reltype_count == 0) ? NULL : reltypes),
													   reltype_count,
													   range);
		new_idx ++;
		if(i < edge_len - 1) {
			children[new_idx + i] = cypher_ast_node_pattern(NULL,
															NULL,
															0,
															NULL,
															NULL,
															0,
															range);
			new_idx ++;
		}
	}

	// add the remaining path elements
	for(uint i = idx + 1; i < path_len + edge_len; i ++) {
		children[new_idx] =
			cypher_ast_clone(cypher_ast_pattern_path_get_element(path, i));
		new_idx ++;
	}

	uint new_path_len = path_len + new_idx - 1;
	cypher_astnode_t *new_path = cypher_ast_pattern_path(children,
														 new_path_len,
														 children,
														 new_path_len,
														 range);
	return new_path;
}

static long long _GetEdgeLength(const cypher_astnode_t *edge) {
	const cypher_astnode_t *range = cypher_ast_rel_pattern_get_varlength(edge);
	if(range == NULL) return -1;

	const cypher_astnode_t *start_node = cypher_ast_range_get_start(range);
	const cypher_astnode_t *end_node = cypher_ast_range_get_end(range);
	if(!(start_node && end_node)) return -1;

	const char *start_str = cypher_ast_integer_get_valuestr(start_node);
	char *endptr;
	long long start = strtoll(start_str, &endptr, 10);
	ASSERT(endptr[0] == '\0');
	// fixed length of 0 or 1 or variable-length traversal
	if(start <= 1) return -1;

	const char *end_str = cypher_ast_integer_get_valuestr(end_node);
	long long end = strtoll(end_str, &endptr, 10);
	ASSERT(endptr[0] == '\0');
	// variable-length traversal
	if(start != end) return -1;

	return start;
}

bool AST_ExpandFixedLengthTraversals
(
	const cypher_astnode_t *root
) {
	bool rewritten = false;

	// collect all patterns
	const cypher_astnode_t **patterns = AST_GetTypedNodes(root, CYPHER_AST_PATTERN);
	uint pattern_count = array_len(patterns);
	// for every pattern
	for(uint i = 0; i < pattern_count; i ++) {
		cypher_astnode_t *pattern = (cypher_astnode_t *)patterns[i];
		uint path_count = cypher_ast_pattern_npaths(pattern);
		// visit every path
		for(uint j = 0; j < path_count; j ++) {
			const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, j);
			uint n = cypher_ast_pattern_path_nelements(path);
			// only visit odd indexes to retrieve edges
			for(uint k = 1; k < n; k += 2) {
				const cypher_astnode_t *edge = cypher_ast_pattern_path_get_element(path, k);
				ASSERT(cypher_astnode_type(edge) == CYPHER_AST_REL_PATTERN);
				long long len = _GetEdgeLength(edge);
				if(len == -1) continue;
				// fixed length traversal, rebuild the path
				cypher_astnode_t *new_path = _ExpandPath(path, edge, k, len, n);

				cypher_ast_free((cypher_astnode_t *)path);
				// update the path in the pattern
				cypher_ast_pattern_set_path(pattern, new_path, j);
				rewritten = true;
				j--;
				break;
			}
		}
	}

	array_free(patterns);

	return rewritten;
}

