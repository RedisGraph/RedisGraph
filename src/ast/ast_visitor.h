/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "ast.h"

typedef struct ast_visitor ast_visitor;

typedef bool (*visit)(const cypher_astnode_t *n, bool start, ast_visitor *visitor);

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

void AST_Visitor_visit
(
	const cypher_astnode_t *node,
	ast_visitor *visitor
);

void AST_Visitor_free
(
	ast_visitor *visitor
);