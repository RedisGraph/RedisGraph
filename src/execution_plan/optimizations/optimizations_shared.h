/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once
#include "../../ast/ast_shared.h"


/* Reverse an inequality symbol so that optimizations can support
 * inequalities with right-hand variables. */
AST_Operator Optimizer_ReverseOp(AST_Operator op);
