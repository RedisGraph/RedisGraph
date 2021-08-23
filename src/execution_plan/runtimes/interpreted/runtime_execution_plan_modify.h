/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "ops/op.h"
#include "rax.h"
#include "cypher-parser.h"

/* Adds operation to execution plan as a child of parent. */
void RT_ExecutionPlan_AddOp(RT_OpBase *parent, RT_OpBase *newOp);