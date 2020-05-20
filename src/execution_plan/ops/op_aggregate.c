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
static Record AggregateConsume(OpBase *opBase);
static OpResult AggregateReset(OpBase *opBase);
static OpBase *AggregateClone(const ExecutionPlan *plan, const OpBase *opBase);
static void AggregateFree(OpBase *opBase);

/* Migrate each expression projected by this operation to either
 * the array of keys or the array of aggregate functions as appropriate. */
static void _migrate_expressions(OpAggregate *op, AR_ExpNode **exps) {
	uint exp_count = array_len(exps);
	op->key_exps = array_new(AR_ExpNode *, 0);
	op->aggregate_exps = array_new(AR_ExpNode *, 1);

	for(uint i = 0; i < exp_count; i++) {
		AR_ExpNode *exp = exps[i];
		if(!AR_EXP_ContainsAggregation(exp)) {
			op->key_exps = array_append(op->key_exps, exp);
		} else {
			op->aggregate_exps = array_append(op->aggregate_exps, exp);
		}
	}

	op->aggregate_count = array_len(op->aggregate_exps);
	op->key_count = array_len(op->key_exps);
}

/* Clone all aggregate expression templates to associate with a new Group. */
static inline AR_ExpNode **_build_aggregate_exps(OpAggregate *op) {
	AR_ExpNode **agg_exps = rm_malloc(op->aggregate_count * sizeof(AR_ExpNode *));

	for(uint i = 0; i < op->aggregate_count; i++) {
		agg_exps[i] = AR_EXP_Clone(op->aggregate_exps[i]);
	}

	return agg_exps;
}

/* Build a new Group key of the SIValue results of non-aggregate expressions. */
static inline SIValue *_build_group_key(OpAggregate *op) {
	SIValue *group_keys = rm_malloc(sizeof(SIValue) * op->key_count);

	for(uint i = 0; i < op->key_count; i++) {
		SIValue key = op->group_keys[i];
		SIValue_Persist(&key);
		group_keys[i] = key;
	}

	return group_keys;
}

static Group *_CreateGroup(OpAggregate *op, Record r) {
	/* Create a new group
	 * Clone group keys. */
	SIValue *group_keys = _build_group_key(op);

	/* Get a fresh copy of aggregation functions. */
	AR_ExpNode **agg_exps = _build_aggregate_exps(op);

	/* There's no need to keep a reference to record if we're not sorting groups. */
	Record cache_record = (op->should_cache_records) ? r : NULL;
	op->group = NewGroup(group_keys, op->key_count, agg_exps, op->aggregate_count, cache_record);

	return op->group;
}

static void _ComputeGroupKey(OpAggregate *op, Record r) {
	for(uint i = 0; i < op->key_count; i++) {
		AR_ExpNode *exp = op->key_exps[i];
		op->group_keys[i] = AR_EXP_Evaluate(exp, r);
	}
}

static void _ComputeGroupKeyStr(OpAggregate *op, char **key) {
	if(op->key_count == 0) {
		*key = rm_strdup("SINGLE_GROUP");
		return;
	}

	// Determine required size for group key string representation.
	size_t key_len = SIValue_StringJoinLen(op->group_keys, op->key_count, ",");
	*key = rm_malloc(sizeof(char) * key_len);
	size_t bytesWritten = 0;
	SIValue_StringJoin(op->group_keys, op->key_count, ",", key, &key_len, &bytesWritten);
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
		goto cleanup;
	}

	// Evaluate non-aggregated fields, see if they match
	// the last accessed group.
	bool reuseLastAccessedGroup = true;
	for(uint i = 0; reuseLastAccessedGroup && i < op->key_count; i++) {
		reuseLastAccessedGroup = (SIValue_Compare(op->group->keys[i], op->group_keys[i], NULL) == 0);
	}

	// See if we can reuse last accessed group.
	if(reuseLastAccessedGroup) return op->group;

	// Can't reuse last accessed group, lookup group by identifier key.
	_ComputeGroupKeyStr(op, &group_key_str);
	op->group = CacheGroupGet(op->groups, group_key_str);
	if(!op->group) {
		// Group does not exists, create it.
		op->group = _CreateGroup(op, r);
		CacheGroupAdd(op->groups, group_key_str, op->group);
	}
cleanup:
	rm_free(group_key_str);
	return op->group;
}

static void _aggregateRecord(OpAggregate *op, Record r) {
	/* Get group */
	Group *group = _GetGroup(op, r);
	assert(group);

	// Aggregate group exps.
	for(uint i = 0; i < op->aggregate_count; i++) {
		AR_ExpNode *exp = group->aggregationFunctions[i];
		AR_EXP_Aggregate(exp, r);
	}

	// Free record.
	OpBase_DeleteRecord(r);
}

/* Returns a record populated with group data. */
static Record _handoff(OpAggregate *op) {
	char *key;
	Group *group;
	if(!CacheGroupIterNext(op->group_iter, &key, &group)) return NULL;

	Record r = OpBase_CreateRecord((OpBase *)op);

	// Add all projected keys to the Record.
	for(uint i = 0; i < op->key_count; i++) {
		int rec_idx = op->record_offsets[i];
		// Non-aggregated expression.
		SIValue res = group->keys[i];
		// Key values are shared with the Record, as they'll be freed with the group cache.
		res = SI_ShareValue(res);
		Record_Add(r, rec_idx, res);
	}

	// Compute the final value of all aggregating expressions and add to the Record.
	for(uint i = 0; i < op->aggregate_count; i++) {
		int rec_idx = op->record_offsets[i + op->key_count];
		AR_ExpNode *exp = group->aggregationFunctions[i];
		AR_EXP_Reduce(exp);
		SIValue res = AR_EXP_Evaluate(exp, NULL);
		Record_AddScalar(r, rec_idx, res);
	}

	return r;
}

OpBase *NewAggregateOp(const ExecutionPlan *plan, AR_ExpNode **exps, bool should_cache_records) {
	OpAggregate *op = rm_malloc(sizeof(OpAggregate));
	op->group = NULL;
	op->group_iter = NULL;
	op->group_keys = NULL;
	op->groups = CacheGroupNew();
	op->should_cache_records = should_cache_records;

	// Migrate each expression to the keys array or the aggregations array as appropriate.
	_migrate_expressions(op, exps);
	array_free(exps);

	// Allocate memory for group keys if we have any non-aggregate expressions.
	if(op->key_count) op->group_keys = rm_malloc(op->key_count * sizeof(SIValue));

	OpBase_Init((OpBase *)op, OPType_AGGREGATE, "Aggregate", NULL, AggregateConsume,
				AggregateReset, NULL, AggregateClone, AggregateFree, false, plan);

	// The projected record will associate values with their resolved name
	// to ensure that space is allocated for each entry.
	op->record_offsets = array_new(uint, op->aggregate_count + op->key_count);
	for(uint i = 0; i < op->key_count; i ++) {
		// Store the index of each key expression.
		int record_idx = OpBase_Modifies((OpBase *)op, op->key_exps[i]->resolved_name);
		op->record_offsets = array_append(op->record_offsets, record_idx);
	}
	for(uint i = 0; i < op->aggregate_count; i ++) {
		// Store the index of each aggregating expression.
		int record_idx = OpBase_Modifies((OpBase *)op, op->aggregate_exps[i]->resolved_name);
		op->record_offsets = array_append(op->record_offsets, record_idx);
	}

	return (OpBase *)op;
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

static OpBase *AggregateClone(const ExecutionPlan *plan, const OpBase *opBase) {
	assert(opBase->type == OPType_AGGREGATE);
	OpAggregate *op = (OpAggregate *)opBase;
	uint key_count = op->key_count;
	uint aggregate_count = op->aggregate_count;
	AR_ExpNode **exps = array_new(AR_ExpNode *, aggregate_count + key_count);
	
	for(uint i = 0; i < key_count; i++) exps = array_append(exps, AR_EXP_Clone(op->key_exps[i]));
	for(uint i = 0; i < aggregate_count; i++) exps = array_append(exps, AR_EXP_Clone(op->aggregate_exps[i]));
	return NewAggregateOp(plan, exps, op->should_cache_records);
}

static void AggregateFree(OpBase *opBase) {
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

