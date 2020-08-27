/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "proc_ctx.h"
#include "../arithmetic/arithmetic_expression.h"

// Perform BFS from a source node.
ProcedureCtx *Proc_BFS_Ctx(AR_ExpNode **args, const char **yields);

