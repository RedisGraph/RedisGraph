/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "ast.h"
#include "../graph/query_graph.h"
#include "../filter_tree/filter_tree.h"

void AST_ConvertFilters(FT_FilterNode **root, const cypher_astnode_t *entity);

FT_FilterNode *AST_BuildFilterTree(AST *ast);
