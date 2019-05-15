/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "ast.h"
#include "../filter_tree/filter_tree.h"

typedef struct FT_FilterNode FT_FilterNode;

FT_FilterNode* CreateCondFilterNode(AST_Operator op);

FT_FilterNode *AppendLeftChild(FT_FilterNode *root, FT_FilterNode *child);
FT_FilterNode *AppendRightChild(FT_FilterNode *root, FT_FilterNode *child);

FT_FilterNode* FilterNode_FromAST(const AST *ast, const cypher_astnode_t *expr);

FT_FilterNode* AST_BuildFilterTree(AST *ast);