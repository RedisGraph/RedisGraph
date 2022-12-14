/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "ast_visitor.h"
#include "util/rmalloc.h"

static bool _default_visit
(
	const cypher_astnode_t *n,
	bool start,
	ast_visitor *visitor,
	bool *err
) {
	ASSERT(n != NULL);

	return true;
}

ast_visitor *AST_Visitor_new
(
	void *ctx
) {
	ast_visitor *visitor = rm_malloc(sizeof(ast_visitor) + sizeof(visit) * 256);
	visitor->ctx = ctx;
	for (uint i = 0; i < 256; i++) {
		visitor->funcs[i] = _default_visit;		
	}
	
	return visitor;
}

void AST_Visitor_register
(
	ast_visitor *visitor,
	cypher_astnode_type_t type,
	visit cb
) {
	ASSERT(cb      != NULL);
	ASSERT(visitor != NULL);

	visitor->funcs[type] = cb;
}

static void _AST_Visitor_visit
(
	const cypher_astnode_t *node,
	ast_visitor *visitor,
	bool *err
) {
	ASSERT(node    != NULL);
	ASSERT(visitor != NULL);

	cypher_astnode_type_t node_type = cypher_astnode_type(node);

	//--------------------------------------------------------------------------
	// opening call
	//--------------------------------------------------------------------------

	// first visit of the node
	if(!visitor->funcs[node_type](node, true, visitor, err) || *err) {
		return;
	}

	//--------------------------------------------------------------------------
	// recall for each child node
	//--------------------------------------------------------------------------

	uint nchildren = cypher_astnode_nchildren(node);
	for (uint i = 0; i < nchildren; i++) {
		_AST_Visitor_visit(cypher_astnode_get_child(node, i), visitor, err);
		if(*err) {
			// child failed, quick return
			return;
		}
	}

	//----------------------------------------------------------------------
	// closing call
	//----------------------------------------------------------------------

	visitor->funcs[node_type](node, false, visitor, err);
}

bool AST_Visitor_visit
(
	const cypher_astnode_t *node,
	ast_visitor *visitor
) {
	bool err = false;
	_AST_Visitor_visit(node, visitor, &err);
	return err;
}

void AST_Visitor_free
(
	ast_visitor *visitor
) {
	ASSERT(visitor != NULL);

	rm_free(visitor);
}
