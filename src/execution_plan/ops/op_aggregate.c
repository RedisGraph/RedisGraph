/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_aggregate.h"
#include "RG.h"
#include "../../util/arr.h"
#include "../../util/rmalloc.h"

// Forward declarations
static void AggregateFree(OpBase *opBase);

// migrate each expression projected by this operation to either
// the array of keys or the array of aggregate functions as appropriate
static void _migrate_expressions(OpAggregate *op, AR_ExpNode **exps) {
	uint exp_count = array_len(exps);
	op->key_exps = array_new(AR_ExpNode *, 0);
	op->aggregate_exps = array_new(AR_ExpNode *, 1);

	for(uint i = 0; i < exp_count; i++) {
		AR_ExpNode *exp = exps[i];
		if(!AR_EXP_ContainsAggregation(exp)) {
			array_append(op->key_exps, exp);
		} else {
			array_append(op->aggregate_exps, exp);
		}
	}

	op->aggregate_count = array_len(op->aggregate_exps);
	op->key_count = array_len(op->key_exps);
}

OpBase *NewAggregateOp(const ExecutionPlan *plan, AR_ExpNode **exps, bool should_cache_records) {
	OpAggregate *op = rm_malloc(sizeof(OpAggregate));
	op->should_cache_records = should_cache_records;

	// migrate each expression to the keys array or the aggregations array as appropriate
	_migrate_expressions(op, exps);
	array_free(exps);

	OpBase_Init((OpBase *)op, OPType_AGGREGATE, "Aggregate", AggregateFree,
		false, plan);

	for(uint i = 0; i < op->key_count; i ++) {
		// Store the index of each key expression.
		OpBase_Modifies((OpBase *)op, op->key_exps[i]->resolved_name);
	}
	for(uint i = 0; i < op->aggregate_count; i ++) {
		// Store the index of each aggregating expression.
		OpBase_Modifies((OpBase *)op, op->aggregate_exps[i]->resolved_name);
	}
	return (OpBase *)op;
}

static void AggregateFree(OpBase *opBase) {
	OpAggregate *op = (OpAggregate *)opBase;
	if(!op) return;

	if(op->key_exps) {
		for(uint i = 0; i < op->key_count; i ++) AR_EXP_Free(op->key_exps[i]);
		array_free(op->key_exps);
		op->key_exps = NULL;
	}

	if(op->aggregate_exps) {
		for(uint i = 0; i < op->aggregate_count; i ++) AR_EXP_Free(op->aggregate_exps[i]);
		array_free(op->aggregate_exps);
		op->aggregate_exps = NULL;
	}
}
