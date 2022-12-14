/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "ast.h"

typedef struct ast_visitor ast_visitor;

typedef bool (*visit)(const cypher_astnode_t *n, bool start, ast_visitor *visitor, bool *err);

typedef struct ast_visitor
{
	void *ctx;
	visit funcs[];
} ast_visitor;

ast_visitor *AST_Visitor_new
(
	void *ctx
);

void AST_Visitor_register
(
	ast_visitor *visitor,
	cypher_astnode_type_t type,
	visit cb
);

bool AST_Visitor_visit
(
	const cypher_astnode_t *node,
	ast_visitor *visitor
);

void AST_Visitor_free
(
	ast_visitor *visitor
);
