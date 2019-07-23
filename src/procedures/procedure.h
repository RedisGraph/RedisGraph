/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "proc_ctx.h"

// Registers procedures.
void Proc_Register();

/* Retrieves procedure, NULL is returned if procedure
 * does not exists, it is the callers responsibility
 * to free returned procedure */
ProcedureCtx *Proc_Get(const char *proc_name);

// Invokes procedure.
ProcedureResult Proc_Invoke(ProcedureCtx *proc, char **args);

/* Single step theough procedure iterator
 * Returns array of key value pairs. */
SIValue *Proc_Step(ProcedureCtx *proc);

/* Resets procedure, restore procedure state to invoked. */
ProcedureResult ProcedureReset(ProcedureCtx *proc);

// Free procedure context.
void Proc_Free(ProcedureCtx *proc);
