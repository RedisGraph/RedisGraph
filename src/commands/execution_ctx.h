/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../ast/ast.h"
#include "../execution_plan/execution_plan.h"

typedef enum {
	EXECUTION_TYPE_INVALID,
	EXECUTION_TYPE_QUERY,
	EXECUTION_TYPE_INDEX_CREATE,
	EXECUTION_TYPE_INDEX_DROP
} ExecutionType;

typedef struct {
	AST *ast;
	ExecutionPlan *exec_plan_template;
} ExecutionCtx;

void ExecutionInformation_FromQuery(const char *query, ExecutionPlan **plan, AST **ast,
									ExecutionType *exec_type, bool *cached);

void ExecutionCtx_Free(ExecutionCtx *ctx);

