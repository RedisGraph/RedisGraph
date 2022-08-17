/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "RG.h"
#include "ast_visitor.h"
#include "util/rmalloc.h"

static bool _default_visit
(
	const cypher_astnode_t *n,
	bool start,
	ast_visitor *visitor
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

void AST_Visitor_visit
(
	const cypher_astnode_t *node,
	ast_visitor *visitor
) {
	ASSERT(node    != NULL);
	ASSERT(visitor != NULL);

	cypher_astnode_type_t node_type = cypher_astnode_type(node);
	if(visitor->funcs[node_type](node, true, visitor)) {
		uint nchildren = cypher_astnode_nchildren(node);
		for (uint i = 0; i < nchildren; i++) {
			AST_Visitor_visit(cypher_astnode_get_child(node, i), visitor);
		}
		visitor->funcs[node_type](node, false, visitor);
	}
}

void AST_Visitor_free
(
	ast_visitor *visitor
) {
	ASSERT(visitor != NULL);

	rm_free(visitor);
}
