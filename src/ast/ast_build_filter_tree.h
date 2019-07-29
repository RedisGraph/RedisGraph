/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "ast.h"
#include "../graph/query_graph.h"
#include "../filter_tree/filter_tree.h"
#include "../execution_plan/record_map.h"

FT_FilterNode *AST_BuildFilterTree(AST *ast, RecordMap *record_map);