/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "ast_visitor.h"
#include "util/rmalloc.h"

static VISITOR_STRATEGY _default_visit
(
	const cypher_astnode_t *n,
	bool start,
	ast_visitor *visitor
) {
	ASSERT(n != NULL);

	return VISITOR_RECURSE;
}

// creates a new visitor
ast_visitor *AST_Visitor_new
(
	void *ctx  // context to attach to the new visitor
) {
	ast_visitor *visitor = rm_malloc(sizeof(ast_visitor) + sizeof(visit) * 256);
	visitor->ctx = ctx;
	for (uint i = 0; i < 256; i++) {
		visitor->funcs[i] = _default_visit;		
	}
	
	return visitor;
}

// registers a function to a visitor
void AST_Visitor_register
(
	ast_visitor *visitor,        // visitor to register the function to
	cypher_astnode_type_t type,  // type of ast-node
	visit cb                     // visit function to register
) {
	ASSERT(cb      != NULL);
	ASSERT(visitor != NULL);

	visitor->funcs[type] = cb;
}

// recursively visit an ast-node
static VISITOR_STRATEGY _AST_Visitor_visit
(
	const cypher_astnode_t *node,
	ast_visitor *visitor
) {
	ASSERT(node    != NULL);
	ASSERT(visitor != NULL);

	cypher_astnode_type_t node_type = cypher_astnode_type(node);

	//--------------------------------------------------------------------------
	// opening call
	//--------------------------------------------------------------------------

	// first visit of the node
	VISITOR_STRATEGY state = visitor->funcs[node_type](node, true, visitor);
	if(state != VISITOR_RECURSE) {
		// do not visit children
		return state;
	}

	//--------------------------------------------------------------------------
	// recall for each child node
	//--------------------------------------------------------------------------

	uint nchildren = cypher_astnode_nchildren(node);
	for (uint i = 0; i < nchildren; i++) {
		if(_AST_Visitor_visit(cypher_astnode_get_child(node, i), visitor) == VISITOR_BREAK) {
			// error occurred, fast fold
			return VISITOR_BREAK;
		}
	}

	//----------------------------------------------------------------------
	// closing call
	//----------------------------------------------------------------------

	return visitor->funcs[node_type](node, false, visitor);
}

// visit an ast-node
void AST_Visitor_visit
(
	const cypher_astnode_t *node,  // node to visit
	ast_visitor *visitor           // visitor
) {
	ASSERT(node    != NULL);
	ASSERT(visitor != NULL);

	_AST_Visitor_visit(node, visitor);
}

// frees a visitor
void AST_Visitor_free
(
	ast_visitor *visitor  // visitor to free
) {
	ASSERT(visitor != NULL);

	rm_free(visitor);
}
