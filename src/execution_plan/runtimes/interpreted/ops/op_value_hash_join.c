/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_value_hash_join.h"
#include "../../../../value.h"
#include "../../../../util/arr.h"
#include "../../../../util/rmalloc.h"
#include "../../../../util/qsort.h"

/* Forward declarations. */
static RT_OpResult ValueHashJoinInit(RT_OpBase *opBase);
static Record ValueHashJoinConsume(RT_OpBase *opBase);
static RT_OpResult ValueHashJoinReset(RT_OpBase *opBase);
static void ValueHashJoinFree(RT_OpBase *opBase);

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
static bool _binarySearchLeftmost(uint *idx, Record *array, uint array_len,
								  int join_key_idx, SIValue v) {
	ASSERT(idx != NULL);

	SIValue x;
	uint pos = 0;
	uint left = 0;
	uint right = array_len;

	while(left < right) {
		pos = (right + left) / 2;
		x = Record_Get(array[pos], join_key_idx);
		if(SIValue_Compare(x, v, NULL) < 0) left = pos + 1;
		else right = pos;
	}

	// Make sure value was found.
	*idx = left;

	if(left == array_len) return false;

	x = Record_Get(array[*idx], join_key_idx);
	// Return false if the value wasn't found or evaluated to NULL.
	int disjointOrNull = 0;
	return (SIValue_Compare(x, v, &disjointOrNull) == 0 &&
			disjointOrNull != COMPARED_NULL);
}

// Performs binary search, returns the rightmost index of a match.
// assuming 'v' exists in 'array'
static bool _binarySearchRightmost(uint *idx, Record *array, uint array_len, int join_key_idx,
								   SIValue v) {
	ASSERT(idx != NULL);

	SIValue x;
	uint pos = 0;
	uint left = 0;
	uint right = array_len;

	while(left < right) {
		pos = (right + left) / 2;
		x = Record_Get(array[pos], join_key_idx);
		if(SIValue_Compare(v, x, NULL) < 0) right = pos;
		else left = pos + 1;
	}

	*idx = right - 1;
	return true;
}

/* Retrive the next intersecting record
 * if such exists, otherwise returns NULL. */
static Record _get_intersecting_record(RT_OpValueHashJoin *op) {
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
static bool _set_intersection_idx(RT_OpValueHashJoin *op, SIValue v) {
	op->intersect_idx = -1;
	op->number_of_intersections = 0;
	uint record_count = array_len(op->cached_records);

	uint leftmost_idx = 0;
	uint rightmost_idx = 0;

	if(!_binarySearchLeftmost(&leftmost_idx, op->cached_records,
							  array_len(op->cached_records), op->join_value_rec_idx, v)) {
		return false;
	}

	/* Value was found
	 * idx points to the first intersecting record.
	 * update number_of_intersections to count how many
	 * records share the same value. */
	op->intersect_idx = leftmost_idx;

	/* Count how many records share the same node.
	 * reduce search space by truncating left bound */
	bool found = _binarySearchRightmost(&rightmost_idx, op->cached_records +
								  leftmost_idx, record_count - leftmost_idx,
								  op->join_value_rec_idx, v);
	UNUSED(found);
	ASSERT(found == true);

	// Compensate index.
	rightmost_idx += leftmost_idx;
	// +1 consider rightmost_idx == leftmost_idx.
	op->number_of_intersections = rightmost_idx - leftmost_idx + 1;
	ASSERT(op->number_of_intersections > 0);

	return true;
}

/* Sorts cached records by joined value. */
void _sort_cached_records(RT_OpValueHashJoin *op) {
	uint idx = op->join_value_rec_idx;
	QSORT(Record, op->cached_records,
		  array_len(op->cached_records), RECORD_SORT_ON_ENTRY);
}

/* Caches all records coming from left branch. */
void _cache_records(RT_OpValueHashJoin *op) {
	ASSERT(op->cached_records == NULL);

	RT_OpBase *left_child = op->op.children[0];
	op->cached_records = array_new(Record, 32);

	Record r = left_child->consume(left_child);
	if(!r) return;

	// As long as there's data coming in from left branch.
	do {
		// Evaluate joined expression.
		SIValue v = AR_EXP_Evaluate(op->op_desc->lhs_exp, r);

		// If the joined value is NULL, it cannot be compared to other values - skip this record.
		if(SIValue_IsNull(v)) continue;

		// Add joined value to record.
		Record_AddScalar(r, op->join_value_rec_idx, v);

		// Cache the record.
		array_append(op->cached_records, r);
	} while((r = left_child->consume(left_child)));
}

/* Creates a new valueHashJoin operation */
RT_OpBase *RT_NewValueHashJoin(const RT_ExecutionPlan *plan, const OpValueHashJoin *op_desc) {
	RT_OpValueHashJoin *op = rm_malloc(sizeof(RT_OpValueHashJoin));
	op->op_desc = op_desc;
	op->rhs_rec = NULL;
	op->intersect_idx = -1;
	op->cached_records = NULL;
	op->number_of_intersections = 0;

	// Set our Op operations
	RT_OpBase_Init((RT_OpBase *)op, (const OpBase *)&op_desc->op,
		ValueHashJoinInit, ValueHashJoinConsume, ValueHashJoinReset,
		ValueHashJoinFree, plan);

	bool aware = RT_OpBase_Aware((RT_OpBase *)op, "pivot", &op->join_value_rec_idx);
	UNUSED(aware);
	ASSERT(aware);
	
	return (RT_OpBase *)op;
}

static RT_OpResult ValueHashJoinInit(RT_OpBase *ctx) {
	ASSERT(ctx->childCount == 2);
	return OP_OK;
}

/* Produce a record by joining
 * records coming from the left and right hand side
 * of this operation. */
static Record ValueHashJoinConsume(RT_OpBase *opBase) {
	RT_OpValueHashJoin *op = (RT_OpValueHashJoin *)opBase;
	RT_OpBase *right_child = op->op.children[1];

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
			// Clone cached record before merging rhs.
			Record c = RT_OpBase_CloneRecord(l);
			Record_Merge(c, op->rhs_rec);
			return c;
		}
	}

	/* If we're here there are no more
	 * left hand side records which intersect with R
	 * discard R. */
	if(op->rhs_rec) RT_OpBase_DeleteRecord(op->rhs_rec);

	/* Try to get new right hand side record
	 * which intersect with a left hand side record. */
	while(true) {
		// Pull from right branch.
		op->rhs_rec = right_child->consume(right_child);
		if(!op->rhs_rec) return NULL;

		// Get value on which we're intersecting.
		SIValue v = AR_EXP_Evaluate(op->op_desc->rhs_exp, op->rhs_rec);

		bool found_intersection = _set_intersection_idx(op, v);
		SIValue_Free(v);
		// No intersection, discard R.
		if(!found_intersection) {
			RT_OpBase_DeleteRecord(op->rhs_rec);
			continue;
		}

		// Found atleast one intersecting record.
		l = _get_intersecting_record(op);

		// Clone cached record before merging rhs.
		Record c = RT_OpBase_CloneRecord(l);
		Record_Merge(c, op->rhs_rec);

		return c;
	}
}

static RT_OpResult ValueHashJoinReset(RT_OpBase *ctx) {
	RT_OpValueHashJoin *op = (RT_OpValueHashJoin *)ctx;
	op->intersect_idx = -1;
	op->number_of_intersections = 0;

	// Clear cached records.
	if(op->rhs_rec) {
		RT_OpBase_DeleteRecord(op->rhs_rec);
		op->rhs_rec = NULL;
	}

	if(op->cached_records) {
		uint record_count = array_len(op->cached_records);
		for(uint i = 0; i < record_count; i++) {
			Record r = op->cached_records[i];
			RT_OpBase_DeleteRecord(r);
		}
		array_free(op->cached_records);
		op->cached_records = NULL;
	}

	return OP_OK;
}

/* Frees ValueHashJoin */
static void ValueHashJoinFree(RT_OpBase *ctx) {
	RT_OpValueHashJoin *op = (RT_OpValueHashJoin *)ctx;
	// Free cached records.
	if(op->rhs_rec) {
		RT_OpBase_DeleteRecord(op->rhs_rec);
		op->rhs_rec = NULL;
	}

	if(op->cached_records) {
		uint record_count = array_len(op->cached_records);
		for(uint i = 0; i < record_count; i++) {
			Record r = op->cached_records[i];
			RT_OpBase_DeleteRecord(r);
		}
		array_free(op->cached_records);
		op->cached_records = NULL;
	}
}
