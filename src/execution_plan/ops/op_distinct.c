/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_distinct.h"
#include "op_project.h"
#include "op_aggregate.h"
#include "xxhash.h"
#include "../../util/arr.h"
#include "../execution_plan_build/execution_plan_modify.h"

/* Forward declarations. */
static OpResult DistinctInit(OpBase *opBase);
static Record DistinctConsume(OpBase *opBase);
static OpBase *DistinctClone(const ExecutionPlan *plan, const OpBase *opBase);
static void DistinctFree(OpBase *opBase);

// compute hash on distinct values
// values that are required to be distinct are located at 'offset'
// positions within the record
static unsigned long long _compute_hash(OpDistinct *op, Record r) {
	// initialize the hash state
	XXH64_state_t state;
	XXH_errorcode res = XXH64_reset(&state, 0);
	ASSERT(res != XXH_ERROR);

	for(uint i = 0; i < op->offset_count; i++) {
		// retrieve the entry at 'idx' as an SIValue
		uint idx = op->offsets[i];
		SIValue v = Record_Get(r, idx);
		// update the hash state with the current value.
		SIValue_HashUpdate(v, &state);
	}

	// finalize the hash
	unsigned long long const hash = XXH64_digest(&state);
	return hash;
}

// compute record offset to distinct values
static void _updateOffsets(OpDistinct *op, Record r) {
	ASSERT(op->aliases != NULL);
	ASSERT(op->offsets != NULL);

	for(uint i = 0; i < op->offset_count; i++) {
		uint offset = Record_GetEntryIdx(r, op->aliases[i]);
		ASSERT(offset != INVALID_INDEX);
		op->offsets[i] = offset;
	}
}

// collect expression aliases on which distinct will be performed
static void LocateDistinctExpressions(OpDistinct *op) {
	// search for projection op
	uint        exp_count  =  0;
	const char  *alias     =  NULL;
	OPType      types[2]   =  {OPType_PROJECT, OPType_AGGREGATE};
	OpBase      *project   =  ExecutionPlan_LocateOpMatchingType((OpBase*)op, types, 2);
	ASSERT(project != NULL);

	OPType t = OpBase_Type(project);
	if(t == OPType_PROJECT) {
		// collect expression aliases from project operation
		OpProject *proj = (OpProject*)project;
		exp_count = proj->exp_count;
		AR_ExpNode **exps = proj->exps;
		op->aliases = rm_malloc(sizeof(char*) * exp_count);

		for(uint i = 0; i < exp_count; i++) {
			alias = exps[i]->resolved_name;
			ASSERT(alias != NULL);
			op->aliases[i] = alias;
		}
	} else {
		// collect expression aliases from aggregate operation
		OpAggregate *proj = (OpAggregate*)project;
		exp_count = proj->key_count + proj->aggregate_count;
		op->aliases = rm_malloc(sizeof(char*) * exp_count);

		AR_ExpNode **exps = proj->key_exps;
		for(uint i = 0; i < proj->key_count; i++) {
			alias = exps[i]->resolved_name;
			ASSERT(alias != NULL);
			op->aliases[i] = alias;
		}

		exps = proj->aggregate_exps;
		for(uint i = 0; i < proj->aggregate_count; i++) {
			alias = exps[i]->resolved_name;
			ASSERT(alias != NULL);
			op->aliases[i + proj->key_count] = alias;
		}
	}

	op->offsets = rm_malloc(sizeof(int) * exp_count);
	op->offset_count = exp_count;
}

OpBase *NewDistinctOp(const ExecutionPlan *plan) {
	OpDistinct *op = rm_malloc(sizeof(OpDistinct));

	op->found           =  raxNew();
	op->mapping         =  NULL;
	op->aliases         =  NULL;
	op->offsets         =  NULL;
	op->offset_count    =  0;

	OpBase_Init((OpBase *)op, OPType_DISTINCT, "Distinct", DistinctInit, DistinctConsume,
				NULL, NULL, DistinctClone, DistinctFree, false, plan);

	return (OpBase *)op;
}

static OpResult DistinctInit(OpBase *opBase) {
	OpDistinct *op = (OpDistinct*)opBase;
	LocateDistinctExpressions(op);
	return OP_OK;
}

static Record DistinctConsume(OpBase *opBase) {
	OpDistinct *op = (OpDistinct *)opBase;
	OpBase *child = op->op.children[0];

	while(true) {
		Record r = OpBase_Consume(child);
		if(!r) return NULL;

		// update offsets if record mapping changed
		// it is possible for the record's mapping to be changed throughtout
		// the execution as this distinct operation might recieve records from
		// different sub execution plans, such as in the case of UNION
		// in which case the distinct values might be located at different offsets
		// within the record and we should adjust accordingly
		if(Record_GetMappings(r) != op->mapping) {
			// record mapping changed, update offsets
			_updateOffsets(op, r);
			// update operation mapping to records mapping
			op->mapping = Record_GetMappings(r);
		}

		unsigned long long const hash = _compute_hash(op, r);
		int is_new = raxInsert(op->found, (unsigned char *) &hash, sizeof(hash), NULL, NULL);
		if(is_new) return r;
		OpBase_DeleteRecord(r);
	}
}

static inline OpBase *DistinctClone(const ExecutionPlan *plan, const OpBase *opBase) {
	ASSERT(opBase->type == OPType_DISTINCT);
	return NewDistinctOp(plan);
}

static void DistinctFree(OpBase *ctx) {
	OpDistinct *op = (OpDistinct *)ctx;
	if(op->found) {
		raxFree(op->found);
		op->found = NULL;
	}

	if(op->aliases) {
		rm_free(op->aliases);
		op->aliases	= NULL;
	}

	if(op->offsets) {
		rm_free(op->offsets);
		op->offsets	= NULL;
	}
}

