/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "ast_rewrite.h"
#include "ast.h"
#include "../query_ctx.h"
#include "../errors.h"
#include "../util/arr.h"
#include "../util/qsort.h"
#include "../util/sds/sds.h"
#include "../procedures/procedure.h"

bool AST_Rewrite
(
	cypher_parse_result_t *result
) {
	bool rewritten = false;
	// retrieve the statement node
	const cypher_astnode_t *statement = cypher_parse_result_get_root(result, 0);
	if(cypher_astnode_type(statement) != CYPHER_AST_STATEMENT) return rewritten;

	// retrieve the root query node from the statement
	const cypher_astnode_t *root = cypher_ast_statement_get_body(statement);
	if(cypher_astnode_type(root) != CYPHER_AST_QUERY) return rewritten;

	rewritten = AST_RewriteStarProjections(root);

	rewritten |= AST_ExpandFixedLengthTraversals(root);

	return rewritten;
}
