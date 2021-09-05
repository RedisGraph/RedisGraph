/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "op.h"
#include "op_sort.h"
#include "op_aggregate.h"
#include "../../../../util/arr.h"
#include "../../../../query_ctx.h"
#include "../../../../util/rmalloc.h"
#include "../../../../grouping/group.h"

/* Forward declarations. */
static Record AggregateConsume(RT_OpBase *opBase);
static RT_OpResult AggregateReset(RT_OpBase *opBase);
static void AggregateFree(RT_OpBase *opBase);

/* Clone all aggregate expression templates to associate with a new Group. */
static inline AR_ExpNode **_build_aggregate_exps(RT_OpAggregate *op) {
	AR_ExpNode **agg_exps = rm_malloc(op->op_desc->aggregate_count * sizeof(AR_ExpNode *));

	for(uint i = 0; i < op->op_desc->aggregate_count; i++) {
		agg_exps[i] = AR_EXP_Clone(op->aggregate_exps[i]);
	}

	return agg_exps;
}

// build a new group key from the SIValue results of non-aggregate expressions
static inline SIValue *_build_group_key(RT_OpAggregate *op) {
	// TODO: might be expensive incase we're generating lots of groups
	SIValue *group_keys = rm_malloc(sizeof(SIValue) * op->op_desc->key_count);

	for(uint i = 0; i < op->op_desc->key_count; i++) {
		SIValue key = SI_TransferOwnership(&op->group_keys[i]);
		SIValue_Persist(&key);
		group_keys[i] = key;
	}

	return group_keys;
}

static Group *_CreateGroup(RT_OpAggregate *op, Record r) {
	// create a new group, clone group keys
	SIValue *group_keys = _build_group_key(op);

	// get a fresh copy of aggregation functions
	AR_ExpNode **agg_exps = _build_aggregate_exps(op);

	// There's no need to keep a reference to record if we're not sorting groups
	Record cache_record = (op->op_desc->should_cache_records) ? r : NULL;
	op->group = NewGroup(group_keys, op->op_desc->key_count, agg_exps,
			op->op_desc->aggregate_count, cache_record);

	return op->group;
}

static void _ComputeGroupKey(RT_OpAggregate *op, Record r) {
	for(uint i = 0; i < op->op_desc->key_count; i++) {
		AR_ExpNode *exp = op->key_exps[i];
		op->group_keys[i] = AR_EXP_Evaluate(exp, r);
	}
}

// compute the hash code for list of SIValues
static XXH64_hash_t _HashCode(const SIValue *v, size_t n) {
	// initialize the hash state
	XXH64_state_t state;
	XXH_errorcode res = XXH64_reset(&state, 0);
	ASSERT(res != XXH_ERROR);

	// update the hash state with the current value.
	for(size_t i = 0; i < n; i++) SIValue_HashUpdate(v[i], &state);

	// finalize the hash
	return XXH64_digest(&state);
}

// retrieves group under which given record belongs to,
// creates group if one doesn't exists
static Group *_GetGroup(RT_OpAggregate *op, Record r) {
	XXH64_hash_t hash;
	bool free_key_exps = true;

	// construct group key
	_ComputeGroupKey(op, r);

	// first group created
	if(!op->group) {
		op->group = _CreateGroup(op, r);
		hash = _HashCode(op->group_keys, op->op_desc->key_count);
		CacheGroupAdd(op->groups, hash, op->group);
		// key expressions are owned by the new group and don't need to be freed
		free_key_exps = false;
		goto cleanup;
	}

	// evaluate non-aggregated fields, see if they match the last accessed group
	bool reuseLastAccessedGroup = true;
	for(uint i = 0; reuseLastAccessedGroup && i < op->op_desc->key_count; i++) {
		reuseLastAccessedGroup =
			(SIValue_Compare(op->group->keys[i], op->group_keys[i], NULL) == 0);
	}

	// see if we can reuse last accessed group
	if(reuseLastAccessedGroup) goto cleanup;

	// can't reuse last accessed group, lookup group by identifier key
	hash = _HashCode(op->group_keys, op->op_desc->key_count);
	op->group = CacheGroupGet(op->groups, hash);
	if(!op->group) {
		// Group does not exists, create it.
		op->group = _CreateGroup(op, r);
		CacheGroupAdd(op->groups, hash, op->group);
		// key expressions are owned by the new group and don't need to be freed
		free_key_exps = false;
	}

cleanup:
	// free the keys that have been computed during this function
	// if they have not been used to build a new group
	if(free_key_exps) {
		for(uint i = 0; i < op->op_desc->key_count; i++) SIValue_Free(op->group_keys[i]);
	}

	return op->group;
}

static void _aggregateRecord(RT_OpAggregate *op, Record r) {
	// get group
	Group *group = _GetGroup(op, r);
	ASSERT(group != NULL);

	// aggregate group exps
	for(uint i = 0; i < op->op_desc->aggregate_count; i++) {
		AR_ExpNode *exp = group->aggregationFunctions[i];
		AR_EXP_Aggregate(exp, r);
	}

	// free record
	RT_OpBase_DeleteRecord(r);
}

// returns a record populated with group data
static Record _handoff(RT_OpAggregate *op) {
	Group *group;
	if(!CacheGroupIterNext(op->group_iter, &group)) return NULL;

	Record r = RT_OpBase_CreateRecord((RT_OpBase *)op);

	// Add all projected keys to the Record.
	for(uint i = 0; i < op->op_desc->key_count; i++) {
		int rec_idx = op->record_offsets[i];
		// Non-aggregated expression.
		SIValue res = group->keys[i];
		// Key values are shared with the Record, as they'll be freed with the group cache.
		res = SI_ShareValue(res);
		Record_Add(r, rec_idx, res);
	}

	// Compute the final value of all aggregating expressions and add to the Record.
	for(uint i = 0; i < op->op_desc->aggregate_count; i++) {
		int rec_idx = op->record_offsets[i + op->op_desc->key_count];
		AR_ExpNode *exp = group->aggregationFunctions[i];

		SIValue res = AR_EXP_Finalize(exp, r);
		Record_AddScalar(r, rec_idx, res);
	}

	return r;
}

RT_OpBase *RT_NewAggregateOp(const RT_ExecutionPlan *plan, const OpAggregate *op_desc) {
	RT_OpAggregate *op = rm_malloc(sizeof(RT_OpAggregate));
	op->op_desc = op_desc;
	array_clone_with_cb(op->key_exps, op_desc->key_exps, AR_EXP_Clone);
	array_clone_with_cb(op->aggregate_exps, op_desc->aggregate_exps, AR_EXP_Clone);
	op->group = NULL;
	op->group_iter = NULL;
	op->group_keys = NULL;
	op->groups = CacheGroupNew();

	// Allocate memory for group keys if we have any non-aggregate expressions.
	if(op_desc->key_count) op->group_keys = rm_malloc(op_desc->key_count * sizeof(SIValue));

	RT_OpBase_Init((RT_OpBase *)op, (const OpBase *)&op_desc->op, NULL, NULL,
		AggregateConsume, AggregateReset, AggregateFree, plan);

	// The projected record will associate values with their resolved name
	// to ensure that space is allocated for each entry.
	op->record_offsets = array_new(uint, op_desc->aggregate_count + op_desc->key_count);
	for(uint i = 0; i < op_desc->key_count; i ++) {
		// Store the index of each key expression.
		uint record_idx;
		bool aware = RT_OpBase_Aware((RT_OpBase *)op, op->key_exps[i]->resolved_name, &record_idx);
		ASSERT(aware);
		array_append(op->record_offsets, record_idx);
	}
	for(uint i = 0; i < op_desc->aggregate_count; i ++) {
		// Store the index of each aggregating expression.
		uint record_idx;
		bool aware = RT_OpBase_Aware((RT_OpBase *)op, op->aggregate_exps[i]->resolved_name, &record_idx);
		ASSERT(aware);
		array_append(op->record_offsets, record_idx);
	}

	return (RT_OpBase *)op;
}

static Record AggregateConsume(RT_OpBase *opBase) {
	RT_OpAggregate *op = (RT_OpAggregate *)opBase;
	if(op->group_iter) return _handoff(op);

	Record r;
	if(op->op.childCount == 0) {
		/* RETURN max (1)
		 * Create a 'fake' record. */
		r = RT_OpBase_CreateRecord(opBase);
		_aggregateRecord(op, r);
	} else {
		RT_OpBase *child = op->op.children[0];
		while((r = RT_OpBase_Consume(child))) _aggregateRecord(op, r);
	}

	op->group_iter = CacheGroupIter(op->groups);
	return _handoff(op);
}

static RT_OpResult AggregateReset(RT_OpBase *opBase) {
	RT_OpAggregate *op = (RT_OpAggregate *)opBase;

	FreeGroupCache(op->groups);
	op->groups = CacheGroupNew();

	if(op->group_iter) {
		CacheGroupIterator_Free(op->group_iter);
		op->group_iter = NULL;
	}

	op->group = NULL;

	return OP_OK;
}

static void AggregateFree(RT_OpBase *opBase) {
	RT_OpAggregate *op = (RT_OpAggregate *)opBase;
	if(!op) return;

	if(op->group_keys) {
		rm_free(op->group_keys);
		op->group_keys = NULL;
	}

	if(op->group_iter) {
		CacheGroupIterator_Free(op->group_iter);
		op->group_iter = NULL;
	}

	if(op->groups) {
		FreeGroupCache(op->groups);
		op->groups = NULL;
	}

	if(op->record_offsets) {
		array_free(op->record_offsets);
		op->record_offsets = NULL;
	}

	if(op->key_exps) {
		for(uint i = 0; i < op->op_desc->key_count; i ++) AR_EXP_Free(op->key_exps[i]);
		array_free(op->key_exps);
		op->key_exps = NULL;
	}

	if(op->aggregate_exps) {
		for(uint i = 0; i < op->op_desc->aggregate_count; i ++) AR_EXP_Free(op->aggregate_exps[i]);
		array_free(op->aggregate_exps);
		op->aggregate_exps = NULL;
	}

	op->group = NULL;
}
