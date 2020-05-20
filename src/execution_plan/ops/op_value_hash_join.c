/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_value_hash_join.h"
#include "../../value.h"
#include "../../util/arr.h"
#include "../../util/rmalloc.h"
#include "../../util/qsort.h"
#include <assert.h>

/* Forward declarations. */
static OpResult ValueHashJoinInit(OpBase *opBase);
static Record ValueHashJoinConsume(OpBase *opBase);
static OpResult ValueHashJoinReset(OpBase *opBase);
static OpBase *ValueHashJoinClone(const ExecutionPlan *plan, const OpBase *opBase);
static void ValueHashJoinFree(OpBase *opBase);

/* Determins order between two records by inspecting
 * element stored at postion idx. */
static bool _record_islt(Record l, Record r, uint idx) {
	SIValue lv = Record_Get(l, idx);
	SIValue rv = Record_Get(r, idx);
	return SIValue_Compare(lv, rv, NULL) < 0;
}

/* `idx` is an actual variable in the caller function.
 * Using it in a macro like this is rather ugly,
 * but the macro passed to QSORT must accept only 2 arguments. */
#define RECORD_SORT_ON_ENTRY(a, b) (_record_islt((*a), (*b), idx))

// Performs binary search, returns the leftmost index of a match.
static int64_t _binarySearchLeftmost(Record *array, int join_key_idx, SIValue v) {
	int64_t recordCount = array_len(array);
	int64_t pos;
	int64_t left = 0;
	int64_t right = recordCount;
	while(left < right) {
		pos = (right + left) / 2;
		SIValue x = Record_Get(array[pos], join_key_idx);
		if(SIValue_Compare(x, v, NULL) < 0) left = pos + 1;
		else right = pos;
	}
	return left;
}

// Performs binary search, returns the rightmost index of a match.
static int64_t _binarySearchRightmost(Record *array, int64_t array_len, int join_key_idx,
									  SIValue v) {
	int64_t pos;
	int64_t left = 0;
	int64_t right = array_len;
	while(left < right) {
		pos = (right + left) / 2;
		SIValue x = Record_Get(array[pos], join_key_idx);
		if(SIValue_Compare(v, x, NULL) < 0) right = pos;
		else left = pos + 1;
	}
	return right - 1;
}

/* Retrive the next intersecting record
 * if such exists, otherwise returns NULL. */
static Record _get_intersecting_record(OpValueHashJoin *op) {
	// No more intersecting records.
	if(op->intersect_idx == -1 ||
	   op->number_of_intersections == 0) return NULL;

	Record cr = op->cached_records[op->intersect_idx];

	// Update intersection trackers.
	op->intersect_idx++;
	op->number_of_intersections--;

	return cr;
}

/* Look up first intersecting cached record CR position.
 * Returns false if no intersecting record is found. */
static bool _set_intersection_idx(OpValueHashJoin *op, SIValue v) {
	op->intersect_idx = -1;
	op->number_of_intersections = 0;
	uint record_count = array_len(op->cached_records);

	int64_t leftmost_idx = _binarySearchLeftmost(op->cached_records, op->join_value_rec_idx, v);
	if(leftmost_idx >= record_count) return false;

	// Make sure value was found.
	Record r = op->cached_records[leftmost_idx];
	SIValue x = Record_Get(r, op->join_value_rec_idx);
	if(SIValue_Compare(x, v, NULL) != 0) return false; // Value wasn't found.

	/* Value was found
	 * idx points to the first intersecting record.
	 * update number_of_intersections to count how many
	 * records share the same value. */
	op->intersect_idx = leftmost_idx;

	/* Count how many records share the same node.
	 * reduce search space by truncating left bound */
	int64_t rightmost_idx = _binarySearchRightmost(op->cached_records + leftmost_idx,
												   record_count - leftmost_idx,
												   op->join_value_rec_idx,
												   v);
	// Compensate index.
	rightmost_idx += leftmost_idx;
	// +1 consider rightmost_idx == leftmost_idx.
	op->number_of_intersections = rightmost_idx - leftmost_idx + 1;
	assert(op->number_of_intersections > 0);

	return true;
}

/* Sorts cached records by joined value. */
void _sort_cached_records(OpValueHashJoin *op) {
	uint idx = op->join_value_rec_idx;
	QSORT(Record, op->cached_records,
		  array_len(op->cached_records), RECORD_SORT_ON_ENTRY);
}

/* Caches all records coming from left branch. */
void _cache_records(OpValueHashJoin *op) {
	assert(op->cached_records == NULL);

	OpBase *left_child = op->op.children[0];
	op->cached_records = array_new(Record, 32);

	Record r = left_child->consume(left_child);
	if(!r) return;

	// As long as there's data coming in from left branch.
	do {
		// Add joined value to record.
		op->cached_records = array_append(op->cached_records, r);

		// Evaluate joined expression.
		SIValue v = AR_EXP_Evaluate(op->lhs_exp, r);
		Record_AddScalar(r, op->join_value_rec_idx, v);
	} while((r = left_child->consume(left_child)));
}

/* String representation of operation */
static int ValueHashJoinToString(const OpBase *ctx, char *buff, uint buff_len) {
	const OpValueHashJoin *op = (const OpValueHashJoin *)ctx;

	char *exp_str = NULL;

	int offset = 0;
	offset += snprintf(buff + offset, buff_len - offset, "%s | ", op->op.name);

	AR_EXP_ToString(op->lhs_exp, &exp_str);
	offset += snprintf(buff + offset, buff_len - offset, "%s", exp_str);
	rm_free(exp_str);

	offset += snprintf(buff + offset, buff_len - offset, " = ");

	AR_EXP_ToString(op->rhs_exp, &exp_str);
	offset += snprintf(buff + offset, buff_len - offset, "%s", exp_str);
	rm_free(exp_str);

	return offset;
}

/* Creates a new valueHashJoin operation */
OpBase *NewValueHashJoin(const ExecutionPlan *plan, AR_ExpNode *lhs_exp, AR_ExpNode *rhs_exp) {
	OpValueHashJoin *op = rm_malloc(sizeof(OpValueHashJoin));
	op->rhs_rec = NULL;
	op->lhs_exp = lhs_exp;
	op->rhs_exp = rhs_exp;
	op->intersect_idx = -1;
	op->cached_records = NULL;
	op->number_of_intersections = 0;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_VALUE_HASH_JOIN, "Value Hash Join", ValueHashJoinInit,
				ValueHashJoinConsume, ValueHashJoinReset, ValueHashJoinToString, ValueHashJoinClone,
				ValueHashJoinFree, false, plan);

	op->join_value_rec_idx = OpBase_Modifies((OpBase *)op, "pivot");
	return (OpBase *)op;
}

static OpResult ValueHashJoinInit(OpBase *ctx) {
	assert(ctx->childCount == 2);
	return OP_OK;
}

/* Produce a record by joining
 * records coming from the left and right hand side
 * of this operation. */
static Record ValueHashJoinConsume(OpBase *opBase) {
	OpValueHashJoin *op = (OpValueHashJoin *)opBase;
	OpBase *right_child = op->op.children[1];

	// Eager, pull from left branch until depleted.
	if(op->cached_records == NULL) {
		_cache_records(op);
		// Sort cache on intersect node ID.
		_sort_cached_records(op);
	}

	/* Try to produce a record:
	 * given a right hand side record R,
	 * evaluate V = exp on R,
	 * see if there are any cached records
	 * which V evaluated to V:
	 * X in cached_records and X[idx] = V
	 * return merged record:
	 * X merged with R. */

	Record l;
	if(op->number_of_intersections > 0) {
		while((l = _get_intersecting_record(op))) {
			/* Merge into cached records to avoid
			 * record extension */
			Record_Merge(&l, op->rhs_rec);
			return OpBase_CloneRecord(l);
		}
	}

	/* If we're here there are no more
	 * left hand side records which intersect with R
	 * discard R. */
	if(op->rhs_rec) OpBase_DeleteRecord(op->rhs_rec);

	/* Try to get new right hand side record
	 * which intersect with a left hand side record. */
	while(true) {
		// Pull from right branch.
		op->rhs_rec = right_child->consume(right_child);
		if(!op->rhs_rec) return NULL;

		// Get value on which we're intersecting.
		SIValue v = AR_EXP_Evaluate(op->rhs_exp, op->rhs_rec);

		bool found_intersection = _set_intersection_idx(op, v);
		SIValue_Free(v);
		// No intersection, discard R.
		if(!found_intersection) {
			OpBase_DeleteRecord(op->rhs_rec);
			continue;
		}

		// Found atleast one intersecting record.
		l = _get_intersecting_record(op);
		Record_Merge(&l, op->rhs_rec);
		return OpBase_CloneRecord(l);
	}
}

static OpResult ValueHashJoinReset(OpBase *ctx) {
	OpValueHashJoin *op = (OpValueHashJoin *)ctx;
	op->intersect_idx = -1;
	op->number_of_intersections = 0;

	// Clear cached records.
	if(op->rhs_rec) {
		OpBase_DeleteRecord(op->rhs_rec);
		op->rhs_rec = NULL;
	}

	if(op->cached_records) {
		uint record_count = array_len(op->cached_records);
		for(uint i = 0; i < record_count; i++) {
			Record r = op->cached_records[i];
			OpBase_DeleteRecord(r);
		}
		array_free(op->cached_records);
		op->cached_records = NULL;
	}

	return OP_OK;
}

static inline OpBase *ValueHashJoinClone(const ExecutionPlan *plan, const OpBase *opBase) {
	assert(opBase->type == OPType_VALUE_HASH_JOIN);
	OpValueHashJoin *op = (OpValueHashJoin *)opBase;
	return NewValueHashJoin(plan, AR_EXP_Clone(op->lhs_exp), AR_EXP_Clone(op->rhs_exp));
}

/* Frees ValueHashJoin */
static void ValueHashJoinFree(OpBase *ctx) {
	OpValueHashJoin *op = (OpValueHashJoin *)ctx;
	// Free cached records.
	if(op->rhs_rec) {
		OpBase_DeleteRecord(op->rhs_rec);
		op->rhs_rec = NULL;
	}

	if(op->cached_records) {
		uint record_count = array_len(op->cached_records);
		for(uint i = 0; i < record_count; i++) {
			Record r = op->cached_records[i];
			OpBase_DeleteRecord(r);
		}
		array_free(op->cached_records);
		op->cached_records = NULL;
	}

	if(op->lhs_exp) {
		AR_EXP_Free(op->lhs_exp);
		op->lhs_exp = NULL;
	}

	if(op->rhs_exp) {
		AR_EXP_Free(op->rhs_exp);
		op->rhs_exp = NULL;
	}
}

