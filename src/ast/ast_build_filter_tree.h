/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "ast.h"
#include "../graph/query_graph.h"
#include "../filter_tree/filter_tree.h"

/* Convert the given AST predicate node and its children into a filter tree. */
void AST_ConvertFilters(FT_FilterNode **root, const cypher_astnode_t *entity);

/* Collect all appropriate predicate nodes from the given AST
 * and convert them into a filter tree. */
FT_FilterNode *AST_BuildFilterTree(AST *ast);

/* Collect all appropriate predicate nodes from the given clauses
 * and convert them into a filter tree. */
FT_FilterNode *AST_BuildFilterTreeFromClauses(const AST *ast, const cypher_astnode_t **clauses,
											  uint count);

