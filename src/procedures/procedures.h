/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "proc_ctx.h"

// perform BFS from a single source node
ProcedureCtx *Proc_BFS_Ctx();

ProcedureCtx *Proc_CDLP_Ctx();

ProcedureCtx *Proc_LabelsCtx();

// lists all graph indices
ProcedureCtx *Proc_IndexesCtx();

ProcedureCtx *Proc_PropKeysCtx();

ProcedureCtx *Proc_PagerankCtx();

ProcedureCtx *Proc_RelationsCtx();

// returns all the procedures in the systems
ProcedureCtx *Proc_ProceduresCtx();

ProcedureCtx *Proc_FulltextDropIdxGen();

ProcedureCtx *Proc_FulltextQueryNodeGen();

ProcedureCtx *Proc_FulltextCreateNodeIdxGen();

