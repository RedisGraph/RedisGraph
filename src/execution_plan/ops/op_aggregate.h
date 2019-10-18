/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "../../redismodule.h"
#include "../../graph/query_graph.h"
#include "../../grouping/group_cache.h"
#include "../../arithmetic/arithmetic_expression.h"

typedef enum {
	AGGREGATED,
	NON_AGGREGATED,
} ExpClassification;

typedef struct {
	OpBase op;
	AR_ExpNode **exps;                             /* Projected expressions (including order exps). */
	uint *record_offsets;                          /* Record IDs for exps and order_exps. */
	AR_ExpNode **non_aggregated_expressions;       /* Array of arithmetic expression. */
	ExpClassification *expression_classification;  /* Classifies each expression as aggregated/not. */
	rax *groups;
	Group *group;                                  /* Last accessed group. */
	SIValue *group_keys;                           /* Array of values composing an aggregated group. */
	CacheGroupIterator *group_iter;
	Record last_record;
	uint exp_count;                                /* Number of projected expressions. */
} OpAggregate;

OpBase *NewAggregateOp(const ExecutionPlan *plan, AR_ExpNode **exps);

