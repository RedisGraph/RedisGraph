/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "ast.h"

// Build a temporary AST with one MATCH clause that holds the given path.
AST *AST_MockMatchPath(AST *master_ast, const cypher_astnode_t *original_path);

// TODO improve name
AST *AST_MockOptionalMatch(AST *master_ast, cypher_astnode_t *clause);

// Free a temporary AST.
void AST_MockFree(AST *ast);

