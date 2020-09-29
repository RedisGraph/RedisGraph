/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "proc_ctx.h"
#include "../arithmetic/arithmetic_expression.h"
#include <sys/types.h>
#include <stdbool.h>


// Registers procedures.
void Proc_Register();

/* Retrieves procedure, NULL is returned if procedure
 * does not exists, it is the callers responsibility
 * to free returned procedure */
ProcedureCtx *Proc_Get(const char *proc_name);

// Invokes procedure.
ProcedureResult Proc_Invoke(ProcedureCtx *proc, const SIValue *args, const char **yield);

/* Single step theough procedure iterator
 * Returns array of key value pairs. */
SIValue *Proc_Step(ProcedureCtx *proc);

/* Resets procedure, restore procedure state to invoked. */
ProcedureResult ProcedureReset(ProcedureCtx *proc);

/* Return number of arguments required by procedure */
uint Procedure_Argc(const ProcedureCtx *proc);

/* Return number of outputs yield by procedure. */
uint Procedure_OutputCount(const ProcedureCtx *proc);

/* Return the name of the procedure's output at position output_idx. */
const char *Procedure_GetOutput(const ProcedureCtx *proc, uint output_idx);

/* Returns true if given output can be yield by procedure */
bool Procedure_ContainsOutput(const ProcedureCtx *proc, const char *output);

/* Returns true if procedure is read-only. */
bool Proc_ReadOnly(const char *proc_name);

// Free procedure context.
void Proc_Free(ProcedureCtx *proc);

