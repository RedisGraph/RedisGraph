/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "proc_ctx.h"

// Returns all the procedures in the systems. This procedure yields the procedures
// names and/or modes (R/W).
ProcedureCtx *Proc_ProceduresCtx();
