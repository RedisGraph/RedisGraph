/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_sort.h"
#include "op_project.h"
#include "op_aggregate.h"
#include "../../util/arr.h"
#include "../../util/qsort.h"
#include "../../util/rmalloc.h"

/* Forward declarations. */
static Record SortConsume(OpBase *opBase);
static OpResult SortReset(OpBase *opBase);
static OpBase *SortClone(const ExecutionPlan *plan, const OpBase *opBase);
static void SortFree(OpBase *opBase);

// Heapsort function to compare two records on a subset of fields.
// Return value similar to strcmp.
static int _record_compare(Record a, Record b, const OpSort *op) {
	uint comparison_count = array_len(op->record_offsets);
	for(uint i = 0; i < comparison_count; i++) {
		SIValue aVal = Record_Get(a, op->record_offsets[i]);
		SIValue bVal = Record_Get(b, op->record_offsets[i]);
		int rel = SIValue_Compare(aVal, bVal, NULL);
		if(rel == 0) continue;  	// Elements are equal; try next ORDER BY element.
		rel *= op->directions[i];   // Flip value for descending order.
		return rel;
	}
	return 0;
}

// Quicksort function to compare two records on a subset of fields.
// Returns true if a is less than b.
static bool _record_islt(Record a, Record b, const OpSort *op) {
	return _record_compare(a, b, op) >
		   0; // Return true if the current left element is less than the right.
}

// Compares two heap record nodes.
static int _heap_elem_compare(const void *A, const void *B, const void *udata) {
	OpSort *op = (OpSort *)udata;
	Record aRec = (Record)A;
	Record bRec = (Record)B;
	return _record_compare(aRec, bRec, op);
}

static void _accumulate(OpSort *op, Record r) {
	if(!op->limit) {
		/* Not using a heap and there's room for record. */
		op->buffer = array_append(op->buffer, r);
		return;
	}

	if(heap_count(op->heap) < op->limit) {
		heap_offer(&op->heap, r);
	} else {
		// No room in the heap, see if we need to replace
		// a heap stored record with the current record.
		if(_heap_elem_compare(heap_peek(op->heap), r, op) > 0) {
			Record replaced = heap_poll(op->heap);
			OpBase_DeleteRecord(replaced);
			heap_offer(&op->heap, r);
		} else {
			OpBase_DeleteRecord(r);
		}
	}
}

static inline Record _handoff(OpSort *op) {
	if(array_len(op->buffer) > 0) return array_pop(op->buffer);
	return NULL;
}

OpBase *NewSortOp(const ExecutionPlan *plan, AR_ExpNode **exps, int *directions, uint limit) {
	OpSort *op = rm_malloc(sizeof(OpSort));
	op->heap = NULL;
	op->buffer = NULL;
	op->limit = limit;
	op->directions = directions;
	op->exps = exps;

	if(op->limit) op->heap = heap_new(_heap_elem_compare, op);
	else op->buffer = array_new(Record, 32);

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_SORT, "Sort", NULL,
				SortConsume, SortReset, NULL, SortClone, SortFree, false, plan);

	uint comparison_count = array_len(exps);
	op->record_offsets = array_new(uint, comparison_count);
	for(uint i = 0; i < comparison_count; i ++) {
		int record_idx;
		assert(OpBase_Aware((OpBase *)op, exps[i]->resolved_name, &record_idx));
		op->record_offsets = array_append(op->record_offsets, record_idx);
	}

	return (OpBase *)op;
}

/* `op` is an actual variable in the caller function. Using it in a
 * macro like this is rather ugly, but the macro passed to QSORT must
 * accept only 2 arguments. */
#define RECORD_SORT(a, b) (_record_islt((*a), (*b), op))

static Record SortConsume(OpBase *opBase) {
	OpSort *op = (OpSort *)opBase;
	Record r = _handoff(op);
	if(r) return r;

	// If we're here, we don't have any records to return
	// try to get records.
	OpBase *child = op->op.children[0];
	bool newData = false;
	while((r = OpBase_Consume(child))) {
		_accumulate(op, r);
		newData = true;
	}
	if(!newData) return NULL;

	if(op->buffer) {
		QSORT(Record, op->buffer, array_len(op->buffer), RECORD_SORT);
	} else {
		// Heap, responses need to be reversed.
		int records_count = heap_count(op->heap);
		op->buffer = array_new(Record, records_count);

		/* Pop items from heap */
		while(records_count > 0) {
			r = heap_poll(op->heap);
			op->buffer = array_append(op->buffer, r);
			records_count--;
		}
	}

	// Pass ordered records downward.
	return _handoff(op);
}

/* Restart iterator */
static OpResult SortReset(OpBase *ctx) {
	OpSort *op = (OpSort *)ctx;
	uint recordCount;

	if(op->heap) {
		recordCount = heap_count(op->heap);
		for(uint i = 0; i < recordCount; i++) {
			Record r = (Record)heap_poll(op->heap);
			OpBase_DeleteRecord(r);
		}
	}

	if(op->buffer) {
		recordCount = array_len(op->buffer);
		for(uint i = 0; i < recordCount; i++) {
			Record r = array_pop(op->buffer);
			OpBase_DeleteRecord(r);
		}
	}

	return OP_OK;
}

static OpBase *SortClone(const ExecutionPlan *plan, const OpBase *opBase) {
	assert(opBase->type == OPType_SORT);
	OpSort *op = (OpSort *)opBase;
	int *directions;
	AR_ExpNode **exps;
	array_clone(directions, op->directions);
	array_clone_with_cb(exps, op->exps, AR_EXP_Clone);
	return NewSortOp(plan, exps, directions, op->limit);
}

/* Frees Sort */
static void SortFree(OpBase *ctx) {
	OpSort *op = (OpSort *)ctx;

	if(op->heap) {
		uint recordCount = heap_count(op->heap);
		for(uint i = 0; i < recordCount; i++) {
			Record r = (Record)heap_poll(op->heap);
			OpBase_DeleteRecord(r);
		}
		heap_free(op->heap);
		op->heap = NULL;
	}

	if(op->buffer) {
		uint recordCount = array_len(op->buffer);
		for(uint i = 0; i < recordCount; i++) {
			Record r = array_pop(op->buffer);
			OpBase_DeleteRecord(r);
		}
		array_free(op->buffer);
		op->buffer = NULL;
	}

	if(op->record_offsets) {
		array_free(op->record_offsets);
		op->record_offsets = NULL;
	}

	if(op->directions) {
		array_free(op->directions);
		op->directions = NULL;
	}

	if(op->exps) {
		uint exps_count = array_len(op->exps);
		for(uint i = 0; i < exps_count; i++) AR_EXP_Free(op->exps[i]);
		array_free(op->exps);
		op->exps = NULL;
	}
}

