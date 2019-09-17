/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_aggregate.h"
#include "op_sort.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"
#include "../../util/rmalloc.h"
#include "../../grouping/group.h"
#include "../../arithmetic/aggregate.h"

/* Forward declarations. */
static OpResult Init(OpBase *opBase);
static Record Consume(OpBase *opBase);
static OpResult Reset(OpBase *opBase);
static void Free(OpBase *opBase);

static AR_ExpNode **_getOrderExpressions(OpBase *op) {
	if(op == NULL) return NULL;
	// No need to look further if we haven't encountered a sort operation
	// before a project/aggregate op
	if(op->type == OPType_PROJECT || op->type == OPType_AGGREGATE) return NULL;

	if(op->type == OPType_SORT) {
		OpSort *sort = (OpSort *)op;
		return sort->exps;
	}

	// We are only interested in a SORT operation if it is a direct parent of the aggregate op
	return NULL;
}

/* Initialize expression_classification, which denotes whether each
 * expression in the RETURN or ORDER segment is an aggregate function.
 * In addition keeps track of non-aggregated expressions in
 * a separate array.*/
static void _classify_expressions(OpAggregate *op) {
	uint exp_count = array_len(op->exps);
	uint order_exp_count = array_len(op->order_exps);
	op->non_aggregated_expressions = array_new(AR_ExpNode *, 0);
	op->expression_classification = rm_malloc(sizeof(ExpClassification) *
											  (exp_count + order_exp_count));

	for(uint i = 0; i < exp_count; i++) {
		AR_ExpNode *exp = op->exps[i];
		if(!AR_EXP_ContainsAggregation(exp, NULL)) {
			op->expression_classification[i] = NON_AGGREGATED;
			op->non_aggregated_expressions = array_append(op->non_aggregated_expressions, exp);
		} else {
			op->expression_classification[i] = AGGREGATED;
		}
	}

	for(uint i = 0; i < order_exp_count; i++) {
		AR_ExpNode *exp = op->order_exps[i];
		if(!AR_EXP_ContainsAggregation(exp, NULL)) {
			op->expression_classification[exp_count + i] = NON_AGGREGATED;
			op->non_aggregated_expressions = array_append(op->non_aggregated_expressions, exp);
		} else {
			op->expression_classification[exp_count + i] = AGGREGATED;
		}
	}
}

static AR_ExpNode **_build_aggregated_expressions(OpAggregate *op) {
	AR_ExpNode **agg_exps = array_new(AR_ExpNode *, 1);

	uint exp_count = array_len(op->exps);
	uint order_exp_count = array_len(op->order_exps);

	for(uint i = 0; i < exp_count; i++) {
		if(op->expression_classification[i] == NON_AGGREGATED) continue;
		AR_ExpNode *exp = AR_EXP_Clone(op->exps[i]);
		agg_exps = array_append(agg_exps, exp);
	}

	for(uint i = 0; i < order_exp_count; i++) {
		if(op->expression_classification[exp_count + i] == NON_AGGREGATED) continue;
		AR_ExpNode *exp = AR_EXP_Clone(op->order_exps[i]);
		agg_exps = array_append(agg_exps, exp);
	}

	return agg_exps;
}

static Group *_CreateGroup(OpAggregate *op, Record r) {
	/* Create a new group
	 * Get a fresh copy of aggregation functions. */
	AR_ExpNode **agg_exps = _build_aggregated_expressions(op);

	/* Clone group keys. */
	uint key_count = array_len(op->non_aggregated_expressions);
	SIValue *group_keys = rm_malloc(sizeof(SIValue) * key_count);

	for(uint i = 0; i < key_count; i++) {
		SIValue key = op->group_keys[i];
		SIValue_Persist(&key);
		group_keys[i] = key;
	}

	/* There's no need to keep a reference to record if we're not sorting groups. */
	if(!op->order_exps) op->group = NewGroup(key_count, group_keys, agg_exps, NULL);
	else op->group = NewGroup(key_count, group_keys, agg_exps, r);

	return op->group;
}

static void _ComputeGroupKey(OpAggregate *op, Record r) {
	uint exp_count = array_len(op->non_aggregated_expressions);

	for(uint i = 0; i < exp_count; i++) {
		AR_ExpNode *exp = op->non_aggregated_expressions[i];
		op->group_keys[i] = AR_EXP_Evaluate(exp, r);
	}
}

static void _ComputeGroupKeyStr(OpAggregate *op, char **key) {
	uint non_agg_exp_count = array_len(op->non_aggregated_expressions);
	if(non_agg_exp_count == 0) {
		*key = rm_strdup("SINGLE_GROUP");
		return;
	}

	// Determine required size for group key string representation.
	size_t key_len = SIValue_StringJoinLen(op->group_keys, non_agg_exp_count, ",");
	*key = rm_malloc(sizeof(char) * key_len);
	size_t bytesWritten = 0;
	SIValue_StringJoin(op->group_keys, non_agg_exp_count, ",", key, &key_len, &bytesWritten);
}

/* Retrieves group under which given record belongs to,
 * creates group if one doesn't exists. */
static Group *_GetGroup(OpAggregate *op, Record r) {
	char *group_key_str;
	// Construct group key.
	_ComputeGroupKey(op, r);

	// First group created.
	if(!op->group) {
		op->group = _CreateGroup(op, r);
		Group_KeyStr(op->group, &group_key_str);
		CacheGroupAdd(op->groups, group_key_str, op->group);
		rm_free(group_key_str);
		return op->group;
	}

	// Evaluate non-aggregated fields, see if they match
	// the last accessed group.
	bool reuseLastAccessedGroup = true;
	uint exp_count = array_len(op->non_aggregated_expressions);
	for(uint i = 0; i < exp_count; i++) {
		if(reuseLastAccessedGroup &&
		   SIValue_Compare(op->group->keys[i], op->group_keys[i], NULL) == 0) {
			reuseLastAccessedGroup = true;
		} else {
			reuseLastAccessedGroup = false;
		}
	}

	// See if we can reuse last accessed group.
	if(reuseLastAccessedGroup) return op->group;

	// Can't reuse last accessed group, lookup group by identifier key.
	_ComputeGroupKeyStr(op, &group_key_str);
	op->group = CacheGroupGet(op->groups, group_key_str);
	if(op->group) {
		rm_free(group_key_str);
		return op->group;
	}

	// Group does not exists, create it.
	op->group = _CreateGroup(op, r);
	CacheGroupAdd(op->groups, group_key_str, op->group);
	rm_free(group_key_str);
	return op->group;
}

static void _aggregateRecord(OpAggregate *op, Record r) {
	/* Get group */
	Group *group = _GetGroup(op, r);
	assert(group);

	// Aggregate group exps.
	uint aggFuncCount = array_len(group->aggregationFunctions);
	for(uint i = 0; i < aggFuncCount; i++) {
		AR_ExpNode *exp = group->aggregationFunctions[i];
		AR_EXP_Aggregate(exp, r);
	}

	/* Free record, incase it is not group representative.
	 * group representative will be freed once group is freed. */
	Record_Free(r);
}

/* Returns a record populated with group data. */
static Record _handoff(OpAggregate *op) {
	char *key;
	Group *group;
	if(!CacheGroupIterNext(op->group_iter, &key, &group)) return NULL;

	uint exp_count = array_len(op->exps);
	uint order_exp_count = array_len(op->order_exps);
	Record r = OpBase_CreateRecord((OpBase *)op);
	// Track the newly-allocated Record so that they may be freed if execution fails.
	OpBase_AddVolatileRecord((OpBase *)op, r);

	// Populate record.
	uint aggIdx = 0; // Index into group aggregated exps.
	uint keyIdx = 0; // Index into group keys.
	SIValue res;

	for(uint i = 0; i < exp_count; i++) {
		if(op->expression_classification[i] == AGGREGATED) {
			// Aggregated expression, get aggregated value.
			AR_ExpNode *exp = group->aggregationFunctions[aggIdx++];
			AR_EXP_Reduce(exp);
			res = AR_EXP_Evaluate(exp, NULL);
			int rec_idx = Record_GetEntryIdx(r, exp->resolved_name);
			Record_AddScalar(r, rec_idx, res);
		} else {
			// Non-aggregated expression.
			res = group->keys[keyIdx++];
			// Key values are shared with the Record, as they'll be freed with the group cache.
			res = SI_ShareValue(res);
			int rec_idx = Record_GetEntryIdx(r, op->exps[i]->resolved_name);
			Record_Add(r, rec_idx, res);
		}
	}

	// Tack order by exps for SORT operation to process.
	for(uint i = 0; i < order_exp_count; i++) {
		if(op->expression_classification[exp_count + i] == AGGREGATED) {
			// Aggregated expression, get aggregated value.
			AR_ExpNode *exp = group->aggregationFunctions[aggIdx++];
			AR_EXP_Reduce(exp);
			res = AR_EXP_Evaluate(exp, NULL);
			int rec_idx = Record_GetEntryIdx(r, exp->resolved_name);
			Record_AddScalar(r, rec_idx, res);
		} else {
			// Non-aggregated expression.
			res = group->keys[keyIdx++];
			// Key values are shared with the Record, as they'll be freed with the group cache.
			res = SI_ShareValue(res);
			int rec_idx = Record_GetEntryIdx(r, op->exps[i]->resolved_name);
			Record_AddScalar(r, rec_idx, res);
		}
	}

	OpBase_RemoveVolatileRecords((OpBase *)op);
	return r;
}

OpBase *NewAggregateOp(const ExecutionPlan *plan, AR_ExpNode **exps) {
	OpAggregate *op = malloc(sizeof(OpAggregate));
	op->ast = QueryCtx_GetAST();
	op->exps = exps;
	op->group = NULL;
	op->order_exps = NULL;
	op->group_iter = NULL;
	op->group_keys = NULL;
	op->expression_classification = NULL;
	op->non_aggregated_expressions = NULL;
	op->groups = CacheGroupNew();

	OpBase_Init((OpBase *)op, OPType_AGGREGATE, "Aggregate", Init, Consume, Reset, NULL, Free, plan);

	return (OpBase *)op;
}

static OpResult Init(OpBase *opBase) {
	OpAggregate *op = (OpAggregate *)opBase;
	uint exp_count = array_len(op->exps);
	for(uint i = 0; i < exp_count; i ++) {
		// The projected record will associate values with their resolved name
		// to ensure that space is allocated for each entry.
		OpBase_Modifies(opBase, op->exps[i]->resolved_name);
	}

	AR_ExpNode **order_exps = _getOrderExpressions(opBase->parent);
	if(order_exps) {
		op->order_exps = order_exps;

		uint order_exp_count = array_len(order_exps);
		for(uint i = 0; i < order_exp_count; i ++) {
			AR_ExpNode *exp = op->order_exps[i];
			OpBase_Modifies(opBase, exp->resolved_name);
		}
	}

	_classify_expressions(op);

	/* Allocate memory for group keys. */
	uint nonAggExpCount = array_len(op->non_aggregated_expressions);
	if(nonAggExpCount) op->group_keys = rm_malloc(sizeof(SIValue) * nonAggExpCount);
	return OP_OK;
}

static Record Consume(OpBase *opBase) {
	OpAggregate *op = (OpAggregate *)opBase;
	OpBase *child = op->op.children[0];

	if(op->group_iter) return _handoff(op);

	Record r;
	while((r = OpBase_Consume(child))) _aggregateRecord(op, r);

	op->group_iter = CacheGroupIter(op->groups);
	return _handoff(op);
}

static OpResult Reset(OpBase *opBase) {
	OpAggregate *op = (OpAggregate *)opBase;

	FreeGroupCache(op->groups);
	op->groups = CacheGroupNew();

	if(op->group_iter) {
		CacheGroupIterator_Free(op->group_iter);
		op->group_iter = NULL;
	}

	return OP_OK;
}

static void Free(OpBase *opBase) {
	OpAggregate *op = (OpAggregate *)opBase;
	if(!op) return;

	if(op->group_keys) {
		rm_free(op->group_keys);
		op->group_keys = NULL;
	}

	if(op->group_iter) {
		CacheGroupIterator_Free(op->group_iter);
		op->group_iter = NULL;
	}

	if(op->expression_classification) {
		rm_free(op->expression_classification);
		op->expression_classification = NULL;
	}

	if(op->non_aggregated_expressions) {
		array_free(op->non_aggregated_expressions);
		op->non_aggregated_expressions = NULL;
	}

	if(op->groups) {
		FreeGroupCache(op->groups);
		op->groups = NULL;
	}
}

