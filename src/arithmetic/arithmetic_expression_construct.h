/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "ast.h"
#include "arithmetic_expression.h"

// Construct arithmetic expression from AST node
AR_ExpNode *AR_EXP_FromASTNode(const cypher_astnode_t *expr);

