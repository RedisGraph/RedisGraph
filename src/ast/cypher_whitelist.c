/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "ast_visitor.h"
#include "../errors.h"
#include "../../deps/libcypher-parser/lib/src/operators.h"

// break visitor traversal, resulting in a fast-fold
static VISITOR_STRATEGY _visit_break
(
	const cypher_astnode_t *n,
	bool start,
	ast_visitor *visitor
) {
	AST_Validation *res = visitor->ctx;
	*res = AST_INVALID;
	Error_UnsupportedASTNodeType(n);
	return VISITOR_BREAK;
}

static VISITOR_STRATEGY _visit_binary_op
(
	const cypher_astnode_t *n,
	bool start,
	ast_visitor *visitor
) {
	AST_Validation *res = visitor->ctx;
	const cypher_operator_t *op = cypher_ast_binary_operator_get_operator(n);
	if(op == CYPHER_OP_SUBSCRIPT ||
	   op == CYPHER_OP_MAP_PROJECTION ||
	   op == CYPHER_OP_REGEX) {
		*res = AST_INVALID;
		Error_UnsupportedASTOperator(op);
		return VISITOR_BREAK;
	}
	return VISITOR_RECURSE;
}

AST_Validation CypherWhitelist_ValidateQuery
(
	const cypher_astnode_t *root
) {
	AST_Validation res = AST_VALID;
	ast_visitor *visitor = AST_Visitor_new(&res);
	AST_Visitor_register(visitor, CYPHER_AST_EXPLAIN_OPTION, _visit_break);
	AST_Visitor_register(visitor, CYPHER_AST_PROFILE_OPTION, _visit_break);
	AST_Visitor_register(visitor, CYPHER_AST_SCHEMA_COMMAND, _visit_break);
	AST_Visitor_register(visitor, CYPHER_AST_CREATE_NODE_PROP_CONSTRAINT, _visit_break);
	AST_Visitor_register(visitor, CYPHER_AST_DROP_NODE_PROP_CONSTRAINT, _visit_break);
	AST_Visitor_register(visitor, CYPHER_AST_CREATE_REL_PROP_CONSTRAINT, _visit_break);
	AST_Visitor_register(visitor, CYPHER_AST_DROP_REL_PROP_CONSTRAINT, _visit_break);
	AST_Visitor_register(visitor, CYPHER_AST_QUERY_OPTION, _visit_break);
	AST_Visitor_register(visitor, CYPHER_AST_USING_PERIODIC_COMMIT, _visit_break);
	AST_Visitor_register(visitor, CYPHER_AST_LOAD_CSV, _visit_break);
	AST_Visitor_register(visitor, CYPHER_AST_START, _visit_break);
	AST_Visitor_register(visitor, CYPHER_AST_START_POINT, _visit_break);
	AST_Visitor_register(visitor, CYPHER_AST_NODE_INDEX_LOOKUP, _visit_break);
	AST_Visitor_register(visitor, CYPHER_AST_NODE_INDEX_QUERY, _visit_break);
	AST_Visitor_register(visitor, CYPHER_AST_NODE_ID_LOOKUP, _visit_break);
	AST_Visitor_register(visitor, CYPHER_AST_ALL_NODES_SCAN, _visit_break);
	AST_Visitor_register(visitor, CYPHER_AST_REL_INDEX_LOOKUP, _visit_break);
	AST_Visitor_register(visitor, CYPHER_AST_REL_INDEX_QUERY, _visit_break);
	AST_Visitor_register(visitor, CYPHER_AST_REL_ID_LOOKUP, _visit_break);
	AST_Visitor_register(visitor, CYPHER_AST_ALL_RELS_SCAN, _visit_break);
	AST_Visitor_register(visitor, CYPHER_AST_MATCH_HINT, _visit_break);
	AST_Visitor_register(visitor, CYPHER_AST_USING_INDEX, _visit_break);
	AST_Visitor_register(visitor, CYPHER_AST_USING_JOIN, _visit_break);
	AST_Visitor_register(visitor, CYPHER_AST_USING_SCAN, _visit_break);
	AST_Visitor_register(visitor, CYPHER_AST_REMOVE_ITEM, _visit_break);
	AST_Visitor_register(visitor, CYPHER_AST_FILTER, _visit_break);  // Deprecated, will not be supported
	AST_Visitor_register(visitor, CYPHER_AST_EXTRACT, _visit_break); // Deprecated, will not be supported
	AST_Visitor_register(visitor, CYPHER_AST_INDEX_NAME, _visit_break);
	AST_Visitor_register(visitor, CYPHER_AST_COMMAND, _visit_break);
	AST_Visitor_register(visitor, CYPHER_AST_FOREACH, _visit_break);
	AST_Visitor_register(visitor, CYPHER_AST_BINARY_OPERATOR, _visit_binary_op);
	AST_Visitor_visit(root, visitor);
	AST_Visitor_free(visitor);
	return res;
}
