/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "ast_mock.h"
#include "../query_ctx.h"
#include "../util/rmalloc.h"

AST *AST_MockMatchPath(AST *master_ast, const cypher_astnode_t *original_path) {
	AST *ast = rm_malloc(sizeof(AST));
	ast->referenced_entities = master_ast->referenced_entities;
	ast->anot_ctx_collection = master_ast->anot_ctx_collection;
	ast->free_root = true;
	ast->limit = NULL;
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

AST *AST_MockOptionalMatch(AST *master_ast, cypher_astnode_t *clause) {
	AST *ast = rm_malloc(sizeof(AST));
	ast->referenced_entities = master_ast->referenced_entities;
	ast->anot_ctx_collection = master_ast->anot_ctx_collection;
	ast->free_root = true;
	ast->limit = UNLIMITED;

	// TODO just need to turn off clause->optional, should be better options.
	struct cypher_input_range range = {};
	const cypher_astnode_t *pattern = cypher_ast_match_get_pattern(clause);
	const cypher_astnode_t *predicate = cypher_ast_match_get_predicate(clause);
	uint child_count = cypher_astnode_nchildren(clause);
	const cypher_astnode_t *children[child_count];
	for(uint i = 0; i < child_count; i ++) children[i] = cypher_astnode_get_child(clause, i);
	cypher_astnode_t *clone = cypher_ast_match(false, pattern, NULL, 0, predicate,
											   (cypher_astnode_t **)children, child_count, range);

	// TODO tmp, bad, unsafe
	ast->root = cypher_ast_query(NULL, 0, &clone, 1, &clone, 1, range);
	// ast->root = cypher_ast_query(NULL, 0, &clause, 1, &clause, 1, range);

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
	// cypher_astnode_free((cypher_astnode_t *)pattern); // TODO tmp
	cypher_astnode_free((cypher_astnode_t *)clause);
	cypher_astnode_free((cypher_astnode_t *)ast->root);
	rm_free(ast);
}

