/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "proc_ctx.h"

// Perform BFS without tracking paths.
ProcedureCtx *Proc_BFS_Ctx(void);

// Perform BFS while tracking the path to each reachable node.
ProcedureCtx *Proc_BFSTree_Ctx(void);

