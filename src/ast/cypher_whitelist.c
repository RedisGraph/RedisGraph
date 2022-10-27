/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "cypher_whitelist.h"
#include "ast_visitor.h"
#include "../errors.h"
#include "../../deps/libcypher-parser/lib/src/operators.h"

static bool _visit
(
	const cypher_astnode_t *n,
	bool start,
	void *ctx
) {
	AST_Validation *res = ctx;
	*res = AST_INVALID;
	Error_UnsupportedASTNodeType(n);
	return false;
}

static bool _visit_binary_op
(
	const cypher_astnode_t *n,
	bool start,
	void *ctx
) {
	AST_Validation *res = ctx;
	const cypher_operator_t *op = cypher_ast_binary_operator_get_operator(n);
	if(op == CYPHER_OP_SUBSCRIPT ||
	   op == CYPHER_OP_MAP_PROJECTION ||
	   op == CYPHER_OP_REGEX) {
		*res = AST_INVALID;
		Error_UnsupportedASTOperator(op);
		return false;
	}
	return true;
}

AST_Validation CypherWhitelist_ValidateQuery(const cypher_astnode_t *root) {
	AST_Validation res = AST_VALID;
	ast_visitor *visitor = AST_Visitor_new(&res);
	AST_Visitor_register(visitor, CYPHER_AST_EXPLAIN_OPTION, _visit);
	AST_Visitor_register(visitor, CYPHER_AST_PROFILE_OPTION, _visit);
	AST_Visitor_register(visitor, CYPHER_AST_SCHEMA_COMMAND, _visit);
	AST_Visitor_register(visitor, CYPHER_AST_CREATE_NODE_PROP_CONSTRAINT, _visit);
	AST_Visitor_register(visitor, CYPHER_AST_DROP_NODE_PROP_CONSTRAINT, _visit);
	AST_Visitor_register(visitor, CYPHER_AST_CREATE_REL_PROP_CONSTRAINT, _visit);
	AST_Visitor_register(visitor, CYPHER_AST_DROP_REL_PROP_CONSTRAINT, _visit);
	AST_Visitor_register(visitor, CYPHER_AST_QUERY_OPTION, _visit);
	AST_Visitor_register(visitor, CYPHER_AST_USING_PERIODIC_COMMIT, _visit);
	AST_Visitor_register(visitor, CYPHER_AST_LOAD_CSV, _visit);
	AST_Visitor_register(visitor, CYPHER_AST_START, _visit);
	AST_Visitor_register(visitor, CYPHER_AST_START_POINT, _visit);
	AST_Visitor_register(visitor, CYPHER_AST_NODE_INDEX_LOOKUP, _visit);
	AST_Visitor_register(visitor, CYPHER_AST_NODE_INDEX_QUERY, _visit);
	AST_Visitor_register(visitor, CYPHER_AST_NODE_ID_LOOKUP, _visit);
	AST_Visitor_register(visitor, CYPHER_AST_ALL_NODES_SCAN, _visit);
	AST_Visitor_register(visitor, CYPHER_AST_REL_INDEX_LOOKUP, _visit);
	AST_Visitor_register(visitor, CYPHER_AST_REL_INDEX_QUERY, _visit);
	AST_Visitor_register(visitor, CYPHER_AST_REL_ID_LOOKUP, _visit);
	AST_Visitor_register(visitor, CYPHER_AST_ALL_RELS_SCAN, _visit);
	AST_Visitor_register(visitor, CYPHER_AST_MATCH_HINT, _visit);
	AST_Visitor_register(visitor, CYPHER_AST_USING_INDEX, _visit);
	AST_Visitor_register(visitor, CYPHER_AST_USING_JOIN, _visit);
	AST_Visitor_register(visitor, CYPHER_AST_USING_SCAN, _visit);
	AST_Visitor_register(visitor, CYPHER_AST_REMOVE_ITEM, _visit);
	AST_Visitor_register(visitor, CYPHER_AST_FILTER, _visit);  // Deprecated, will not be supported
	AST_Visitor_register(visitor, CYPHER_AST_EXTRACT, _visit); // Deprecated, will not be supported
	AST_Visitor_register(visitor, CYPHER_AST_INDEX_NAME, _visit);
	AST_Visitor_register(visitor, CYPHER_AST_COMMAND, _visit);
	AST_Visitor_register(visitor, CYPHER_AST_MAP_PROJECTION_ALL_PROPERTIES, _visit);
	AST_Visitor_register(visitor, CYPHER_AST_BINARY_OPERATOR, _visit_binary_op);
	AST_Visitor_visit(root, visitor);
	AST_Visitor_free(visitor);
	return res;
}
