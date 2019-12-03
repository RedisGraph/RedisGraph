/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "ast_mock.h"
#include "../query_ctx.h"
#include "../util/rmalloc.h"

AST *AST_MockMatchPattern(AST *master_ast, const cypher_astnode_t *original_path) {
	AST *ast = rm_malloc(sizeof(AST));
	ast->referenced_entities = master_ast->referenced_entities;
	ast->anot_ctx_collection = master_ast->anot_ctx_collection;
	ast->free_root = true;
	ast->limit = UNLIMITED;
	struct cypher_input_range range = {};

	// Reuse the input path directly. We cannot clone, as this causes annotations (entity names) to be lost.
	// TODO consider updating parser to improve this.
	cypher_astnode_t *path = (cypher_astnode_t *)original_path;

	// Build a pattern comprised of the input path.
	cypher_astnode_t *pattern = cypher_ast_pattern(&path, 1, &path, 1, range);

	// Build a new match clause that holds this pattern.
	cypher_astnode_t *match_clause = cypher_ast_match(false, pattern, NULL, 0, NULL, &pattern, 1,
													  range);

	// Build a query node holding this clause.
	ast->root = cypher_ast_query(NULL, 0, &match_clause, 1, &match_clause, 1, range);

	QueryCtx_SetAST(ast); // Update the TLS.

	return ast;
}

void AST_MockFree(AST *ast) {
	/* When freeing the mock AST, we have to be careful to not free the shared path
	 * or its annotations. We'll free every surrounding layer explicitly - the MATCH
	 * pattern, the MATCH clause, and finally the AST root. */
	const cypher_astnode_t *clause = cypher_ast_query_get_clause(ast->root, 0);
	assert(cypher_astnode_type(clause) == CYPHER_AST_MATCH);
	const cypher_astnode_t *pattern = cypher_ast_match_get_pattern(clause);
	cypher_astnode_free((cypher_astnode_t *)pattern);
	cypher_astnode_free((cypher_astnode_t *)clause);
	cypher_astnode_free((cypher_astnode_t *)ast->root);
	rm_free(ast);
}

