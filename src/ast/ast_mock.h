/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "ast.h"

// Build a temporary AST with one MATCH clause that holds the given path or pattern.
AST *AST_MockMatchClause(AST *master_ast, cypher_astnode_t *node, bool node_is_pattern);

// Free a temporary AST.
void AST_MockFree(AST *ast, bool free_pattern);

