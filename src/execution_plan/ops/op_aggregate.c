/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
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
static OpResult AggregateInit(OpBase *opBase);
static Record AggregateConsume(OpBase *opBase);
static OpResult AggregateReset(OpBase *opBase);
static OpBase *AggregateClone(const ExecutionPlan *plan, const OpBase *opBase);
static void AggregateFree(OpBase *opBase);

/* Initialize expression_classification, which denotes whether each
 * expression in the RETURN or ORDER segment is an aggregate function.
 * In addition keeps track of non-aggregated expressions in
 * a separate array.*/
static void _classify_expressions(OpAggregate *op) {
	op->non_aggregated_expressions = array_new(AR_ExpNode *, 0);

	op->expression_classification = rm_malloc(sizeof(ExpClassification) * op->exp_count);

	for(uint i = 0; i < op->exp_count; i++) {
		AR_ExpNode *exp = op->exps[i];
		if(!AR_EXP_ContainsAggregation(exp)) {
			op->expression_classification[i] = NON_AGGREGATED;
			op->non_aggregated_expressions = array_append(op->non_aggregated_expressions, exp);
		} else {
			op->expression_classification[i] = AGGREGATED;
		}
	}
}

static AR_ExpNode **_build_aggregated_expressions(OpAggregate *op) {
	AR_ExpNode **agg_exps = array_new(AR_ExpNode *, 1);

	for(uint i = 0; i < op->exp_count; i++) {
		if(op->expression_classification[i] == NON_AGGREGATED) continue;
		AR_ExpNode *exp = AR_EXP_Clone(op->exps[i]);
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
	Record cache_record = (op->should_cache_records) ? r : NULL;
	op->group = NewGroup(key_count, group_keys, agg_exps, cache_record);

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
	OpBase_DeleteRecord(r);
}

/* Returns a record populated with group data. */
static Record _handoff(OpAggregate *op) {
	char *key;
	Group *group;
	if(!CacheGroupIterNext(op->group_iter, &key, &group)) return NULL;

	Record r = OpBase_CreateRecord((OpBase *)op);
	/* In this function, we're only evaluating aggregate function calls, which do not
	 * currently raise exceptions. As such, we don't need to register volatile Records,
	 * though this may not be true in the future. */
	// OpBase_AddVolatileRecord((OpBase *)op, r);

	// Populate record.
	uint aggIdx = 0; // Index into group aggregated exps.
	uint keyIdx = 0; // Index into group keys.
	SIValue res;

	for(uint i = 0; i < op->exp_count; i++) {
		int rec_idx = op->record_offsets[i];
		if(op->expression_classification[i] == AGGREGATED) {
			// Aggregated expression, get aggregated value.
			AR_ExpNode *exp = group->aggregationFunctions[aggIdx++];
			AR_EXP_Reduce(exp);
			res = AR_EXP_Evaluate(exp, NULL);
			Record_AddScalar(r, rec_idx, res);
		} else {
			// Non-aggregated expression.
			res = group->keys[keyIdx++];
			// Key values are shared with the Record, as they'll be freed with the group cache.
			res = SI_ShareValue(res);
			Record_Add(r, rec_idx, res);
		}
	}

	// OpBase_RemoveVolatileRecords((OpBase *)op); // Not currently necessary, as described above.
	return r;
}

OpBase *NewAggregateOp(const ExecutionPlan *plan, AR_ExpNode **exps, bool should_cache_records) {
	OpAggregate *op = rm_malloc(sizeof(OpAggregate));
	op->exps = exps;
	op->group = NULL;
	op->group_iter = NULL;
	op->group_keys = NULL;
	op->expression_classification = NULL;
	op->non_aggregated_expressions = NULL;
	op->groups = CacheGroupNew();
	op->exp_count = array_len(exps);
	op->record_offsets = array_new(uint, op->exp_count);
	op->should_cache_records = should_cache_records;

	OpBase_Init((OpBase *)op, OPType_AGGREGATE, "Aggregate", AggregateInit, AggregateConsume,
				AggregateReset, NULL, AggregateClone, AggregateFree, false, plan);

	for(uint i = 0; i < op->exp_count; i ++) {
		// The projected record will associate values with their resolved name
		// to ensure that space is allocated for each entry.
		int record_idx = OpBase_Modifies((OpBase *)op, op->exps[i]->resolved_name);
		op->record_offsets = array_append(op->record_offsets, record_idx);
	}

	return (OpBase *)op;
}

static OpResult AggregateInit(OpBase *opBase) {
	OpAggregate *op = (OpAggregate *)opBase;

	// Determine whether each expression is an aggregate function or not.
	_classify_expressions(op);

	/* Allocate memory for group keys. */
	uint nonAggExpCount = array_len(op->non_aggregated_expressions);
	if(nonAggExpCount) op->group_keys = rm_malloc(sizeof(SIValue) * nonAggExpCount);
	return OP_OK;
}

static Record AggregateConsume(OpBase *opBase) {
	OpAggregate *op = (OpAggregate *)opBase;
	if(op->group_iter) return _handoff(op);

	Record r;
	if(op->op.childCount == 0) {
		/* RETURN max (1)
		 * Create a 'fake' record. */
		r = OpBase_CreateRecord(opBase);
		_aggregateRecord(op, r);
	} else {
		OpBase *child = op->op.children[0];
		while((r = OpBase_Consume(child))) _aggregateRecord(op, r);
	}

	op->group_iter = CacheGroupIter(op->groups);
	return _handoff(op);
}

static OpResult AggregateReset(OpBase *opBase) {
	OpAggregate *op = (OpAggregate *)opBase;

	FreeGroupCache(op->groups);
	op->groups = CacheGroupNew();

	if(op->group_iter) {
		CacheGroupIterator_Free(op->group_iter);
		op->group_iter = NULL;
	}

	op->group = NULL;

	return OP_OK;
}

static inline OpBase *AggregateClone(const ExecutionPlan *plan, const OpBase *opBase) {
	OpAggregate *op = (OpAggregate *)opBase;
	AR_ExpNode **exps_clone;
	array_clone_with_cb(exps_clone, op->exps, AR_EXP_Clone);
	return NewAggregateOp(plan, exps_clone, op->should_cache_records);
}

static void AggregateFree(OpBase *opBase) {
	OpAggregate *op = (OpAggregate *)opBase;
	if(!op) return;

	if(op->exps) {
		for(uint i = 0; i < op->exp_count; i ++) AR_EXP_Free(op->exps[i]);
		array_free(op->exps);
		op->exps = NULL;
	}

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

	if(op->record_offsets) {
		array_free(op->record_offsets);
		op->record_offsets = NULL;
	}

	op->group = NULL;
}

