/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_cartesian_product.h"

/* Forward declarations. */
static OpResult CartesianProductInit(OpBase *opBase);
static Record CartesianProductConsume(OpBase *opBase);
static OpResult CartesianProductReset(OpBase *opBase);
static void CartesianProductFree(OpBase *opBase);

OpBase *NewCartesianProductOp(const ExecutionPlan *plan) {
	CartesianProduct *op = malloc(sizeof(CartesianProduct));
	op->init = true;
	op->r = NULL;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_CARTESIAN_PRODUCT, "Cartesian Product", CartesianProductInit,
				CartesianProductConsume, CartesianProductReset, NULL, CartesianProductFree, plan, false);
	return (OpBase *)op;
}

static void _ResetStreams(CartesianProduct *cp, int streamIdx) {
	// Reset each child stream, Reset propagates upwards.
	for(int i = 0; i < streamIdx; i++) OpBase_PropagateReset(cp->op.children[i]);
}

static int _PullFromStreams(CartesianProduct *op) {
	for(int i = 1; i < op->op.childCount; i++) {
		OpBase *child = op->op.children[i];
		Record childRecord = OpBase_Consume(child);

		if(childRecord) {
			Record_TransferEntries(&op->r, childRecord);
			Record_Free(childRecord);
			/* Managed to get new data
			 * Reset streams [0-i] */
			_ResetStreams(op, i);

			// Pull from resetted streams.
			for(int j = 0; j < i; j++) {
				child = op->op.children[j];
				childRecord = OpBase_Consume(child);
				if(childRecord) {
					Record_TransferEntries(&op->r, childRecord);
					Record_Free(childRecord);
				} else {
					return 0;
				}
			}
			// Ready to continue.
			return 1;
		}
	}

	/* If we're here, then we didn't manged to get new data.
	 * Last stream depleted. */
	return 0;
}

static OpResult CartesianProductInit(OpBase *opBase) {
	CartesianProduct *op = (CartesianProduct *)opBase;
	op->r = OpBase_CreateRecord((OpBase *)op);
	return OP_OK;
}

static Record CartesianProductConsume(OpBase *opBase) {
	CartesianProduct *op = (CartesianProduct *)opBase;
	OpBase *child;
	Record childRecord;

	if(op->init) {
		op->init = false;

		for(int i = 0; i < op->op.childCount; i++) {
			child = op->op.children[i];
			childRecord = OpBase_Consume(child);
			if(!childRecord) return NULL;
			Record_TransferEntries(&op->r, childRecord);
			Record_Free(childRecord);
		}
		return Record_Clone(op->r);
	}

	// Pull from first stream.
	child = op->op.children[0];
	childRecord = OpBase_Consume(child);

	if(childRecord) {
		// Managed to get data from first stream.
		Record_TransferEntries(&op->r, childRecord);
		Record_Free(childRecord);
	} else {
		// Failed to get data from first stream,
		// try pulling other streams for data.
		if(!_PullFromStreams(op)) return NULL;
	}

	// Pass down a clone of record.
	return Record_Clone(op->r);
}

static OpResult CartesianProductReset(OpBase *opBase) {
	CartesianProduct *op = (CartesianProduct *)opBase;
	op->init = true;
	return OP_OK;
}

static void CartesianProductFree(OpBase *opBase) {
	CartesianProduct *op = (CartesianProduct *)opBase;
	if(op->r) {
		Record_Free(op->r);
		op->r = NULL;
	}
}

