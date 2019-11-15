/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "ast_mock.h"
#include "../query_ctx.h"
#include "../util/rmalloc.h"

AST *AST_MockMatchPattern(AST *master_ast, const cypher_astnode_t *original_path) {
	// Duplicate of AST_NewSegment logic
	AST *ast = rm_malloc(sizeof(AST));
	ast->anot_ctx_collection = master_ast->anot_ctx_collection;
	ast->free_root = true;
	ast->limit = UNLIMITED;
	struct cypher_input_range range = {};

	// Reuse the input path.
	// TODO cloning loses annotations (names). Does reusing the original introduce a memory leak?
	cypher_astnode_t *path = (cypher_astnode_t *)original_path;

	// Build a pattern comprised of 1 path, the clone.
	cypher_astnode_t *pattern = cypher_ast_pattern(&path, 1, &path, 1, range);

	// Build a new match clause that holds this pattern.
	cypher_astnode_t *match_clause = cypher_ast_match(false, pattern, NULL, 0, NULL, &pattern, 1,
													  range);

	// Build a query node holding this clause.
	ast->root = cypher_ast_query(NULL, 0, &match_clause, 1, &match_clause, 1, range);

	QueryCtx_SetAST(ast); // Update the TLS.

	/* TODO don't have access to the projected entities, so in a query like:
	   MATCH (a:A), (b:B) MERGE (a)-[r:TYPE]->(b) RETURN r
	   We won't populate 'r' (which is a problem).
	   Testing inheriting the reference map, but I'm not sure if this will work,
	   we might encounter issues thinking variables resolved later in the query
	   are resolved now. */
	ast->referenced_entities = master_ast->referenced_entities;
	// AST_BuildReferenceMap(ast, NULL); // Build the map of referenced entities.

	// TODO Add Argument variables to map?

	return ast;
}

