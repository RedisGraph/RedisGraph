/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "op_cartesian_product.h"

/* Forward declarations. */
static RT_OpResult CartesianProductInit(RT_OpBase *opBase);
static Record CartesianProductConsume(RT_OpBase *opBase);
static RT_OpResult CartesianProductReset(RT_OpBase *opBase);
static void CartesianProductFree(RT_OpBase *opBase);

RT_OpBase *RT_NewCartesianProductOp(const RT_ExecutionPlan *plan, const CartesianProduct *op_desc) {
	RT_CartesianProduct *op = rm_malloc(sizeof(RT_CartesianProduct));
	op->op_desc = op_desc;
	op->init = true;
	op->r = NULL;

	// Set our Op operations
	RT_OpBase_Init((RT_OpBase *)op, (const OpBase *)&op_desc->op, NULL,
		CartesianProductInit, CartesianProductConsume, CartesianProductReset,
		CartesianProductFree, plan);
	return (RT_OpBase *)op;
}

static void _ResetStreams(RT_CartesianProduct *cp, int streamIdx) {
	// Reset each child stream, Reset propagates upwards.
	for(int i = 0; i < streamIdx; i++) RT_OpBase_PropagateReset(cp->op.children[i]);
}

static int _PullFromStreams(RT_CartesianProduct *op) {
	for(int i = 1; i < op->op.childCount; i++) {
		RT_OpBase *child = op->op.children[i];
		Record childRecord = RT_OpBase_Consume(child);

		if(childRecord) {
			Record_TransferEntries(&op->r, childRecord);
			RT_OpBase_DeleteRecord(childRecord);
			/* Managed to get new data
			 * Reset streams [0-i] */
			_ResetStreams(op, i);

			// Pull from resetted streams.
			for(int j = 0; j < i; j++) {
				child = op->op.children[j];
				childRecord = RT_OpBase_Consume(child);
				if(childRecord) {
					Record_TransferEntries(&op->r, childRecord);
					RT_OpBase_DeleteRecord(childRecord);
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

static RT_OpResult CartesianProductInit(RT_OpBase *opBase) {
	RT_CartesianProduct *op = (RT_CartesianProduct *)opBase;
	op->r = RT_OpBase_CreateRecord((RT_OpBase *)op);
	return OP_OK;
}

static Record CartesianProductConsume(RT_OpBase *opBase) {
	RT_CartesianProduct *op = (RT_CartesianProduct *)opBase;
	RT_OpBase *child;
	Record childRecord;

	if(op->init) {
		op->init = false;

		for(int i = 0; i < op->op.childCount; i++) {
			child = op->op.children[i];
			childRecord = RT_OpBase_Consume(child);
			if(!childRecord) return NULL;
			Record_TransferEntries(&op->r, childRecord);
			RT_OpBase_DeleteRecord(childRecord);
		}
		return RT_OpBase_CloneRecord(op->r);
	}

	// Pull from first stream.
	child = op->op.children[0];
	childRecord = RT_OpBase_Consume(child);

	if(childRecord) {
		// Managed to get data from first stream.
		Record_TransferEntries(&op->r, childRecord);
		RT_OpBase_DeleteRecord(childRecord);
	} else {
		// Failed to get data from first stream,
		// try pulling other streams for data.
		if(!_PullFromStreams(op)) return NULL;
	}

	// Pass down a clone of record.
	return RT_OpBase_CloneRecord(op->r);
}

static RT_OpResult CartesianProductReset(RT_OpBase *opBase) {
	RT_CartesianProduct *op = (RT_CartesianProduct *)opBase;
	op->init = true;
	return OP_OK;
}

static void CartesianProductFree(RT_OpBase *opBase) {
	RT_CartesianProduct *op = (RT_CartesianProduct *)opBase;
	if(op->r) {
		RT_OpBase_DeleteRecord(op->r);
		op->r = NULL;
	}
}
