/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "ast_visitor.h"
#include "util/rmalloc.h"
#include "ast_validations.h"

// default visit function
static VISITOR_STRATEGY _default_visit
(
	const cypher_astnode_t *n,
	bool start,
	void *ctx
) {
	ASSERT(n != NULL);

	return VISITOR_RECURSE;
}

// recursively visit an ast-node
static VISITOR_STRATEGY _AST_Visitor_visit
(
	const cypher_astnode_t *node,
	ast_visitor *visitor
) {
	ASSERT(node    != NULL);
	ASSERT(visitor != NULL);

	// visitation context
	void *ctx = visitor->ctx;

	// get ast node type
	cypher_astnode_type_t node_type = cypher_astnode_type(node);

	// TODO: make sure (DEBUG) access to mapping[node_type]
	// is valid
	ASSERT(node_type < visitor->n..);

	//--------------------------------------------------------------------------
	// opening call
	//--------------------------------------------------------------------------

	// first visit of the node
	VISITOR_STRATEGY state = visitor->mapping[node_type](node, true, ctx);
	if(state != VISITOR_RECURSE) {
		// do not visit children
		return state;
	}

	//--------------------------------------------------------------------------
	// recall for each child node
	//--------------------------------------------------------------------------

	uint nchildren = cypher_astnode_nchildren(node);
	for (uint i = 0; i < nchildren; i++) {
		if(_AST_Visitor_visit(cypher_astnode_get_child(node, i), ctx) == VISITOR_BREAK) {
			// error occurred, fast fold
			return VISITOR_BREAK;
		}
	}

	//--------------------------------------------------------------------------
	// closing call
	//--------------------------------------------------------------------------

	return visitor->mapping[node_type](node, false, ctx);
}

// creates a new visitor
ast_visitor *AST_Visitor_new
(
	void *ctx,                   // context to attach to the new visitor
	ast_visitor_mapping mapping  // mapping between ast-node-types to visiting functions
) {
	ast_visitor *visitor = rm_malloc(sizeof(ast_visitor));
	visitor->ctx     = ctx;
	visitor->mapping = mapping;

	return visitor;
}

// visit an ast-node
void AST_Visitor_visit
(
	const cypher_astnode_t *root,  // node to visit
	ast_visitor *visitor           // visitor
) {
	ASSERT(node    != NULL);
	ASSERT(visitor != NULL);

	_AST_Visitor_visit(node, visitor);
}

// registers a function to a mapping
void AST_Visitor_mapping_register
(
	ast_visitor_mapping mapping, // visitor to register the function to
	cypher_astnode_type_t type,  // type of ast-node
	visit cb                     // visit function to register
) {
	ASSERT(cb      != NULL);
	ASSERT(mapping != NULL);
	ASSERT(mapping[type] == _default_visit);  // override default cb

	mapping[type] = cb;
}

// frees a visitor
void AST_Visitor_free
(
	ast_visitor *visitor  // visitor to free
) {
	ASSERT(visitor != NULL);

	rm_free(visitor);
}

