/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "op_call_subquery.h"

// forward declarations
static void CallSubqueryFree(OpBase *opBase);
static OpResult CallSubqueryInit(OpBase *opBase);
static Record CallSubqueryConsume(OpBase *opBase);
static OpResult CallSubqueryReset(OpBase *opBase);
static Record CallSubqueryConsumeEager(OpBase *opBase);
static OpBase *CallSubqueryClone(const ExecutionPlan *plan, const OpBase *opBase);

// creates a new CallSubquery operation
OpBase *NewCallSubqueryOp
(
	const ExecutionPlan *plan,  // execution plan
    bool is_eager,              // if an updating clause lies in the body, eagerly consume the records
    bool is_returning           // is the subquery returning or unit
) {
    OpCallSubquery *op = rm_calloc(1, sizeof(OpCallSubquery));

	op->r             = NULL;
	op->lhs           = NULL;
    op->body          = NULL;
	op->first         = true;
	op->records       = NULL;
    op->argument      = NULL;
	op->is_eager      = is_eager;
    op->is_returning  = is_returning;
	op->argument_list = NULL;

    // set the consume function according to eagerness of the op
    Record (*consumeFunc)(OpBase *opBase) = is_eager ? CallSubqueryConsumeEager :
                                        CallSubqueryConsume;

    OpBase_Init((OpBase *)op, OPType_CallSubquery, "CallSubquery", CallSubqueryInit,
			consumeFunc, CallSubqueryReset, NULL, CallSubqueryClone, CallSubqueryFree,
			false, plan);

	return (OpBase *)op;
}

static OpResult CallSubqueryInit
(
    OpBase *opBase  // CallSubquery operation to initialize
) {
    OpCallSubquery *op = (OpCallSubquery *)opBase;

	// set the lhs (supplier) branch to be the first child, and rhs branch
    // (body) to be the second
    if(op->op.childCount == 2) {
        op->lhs = op->op.children[0];
        op->body = op->op.children[1];
    } else {
        op->body = op->op.children[0];
    }

	// search for the ArgumentList\Argument op, depending if the op is eager
    // the second will stay NULL
    // find the deepest operation
    OpBase *deepest = op->body;
    while(deepest->childCount > 0) {
        deepest = deepest->children[0];
    }
    if(op->is_eager) {
        op->argument_list = (ArgumentList *)deepest;
        // validate found operation type, expecting ArgumentList
        ASSERT(OpBase_Type((const OpBase *)op->argument_list) == OPType_ARGUMENT_LIST);
    } else {
        // search for Argument op
        op->argument = (Argument *)deepest;
        ASSERT(OpBase_Type((const OpBase *)op->argument) == OPType_ARGUMENT);
    }

    return OP_OK;
}

// passes a record to the parent op.
// if the subquery is non-returning (unit), all the records have already been
// consumed from the body, so that we only need to pop the records
// the returning subquery case: TBD
static Record _handoff_eager(OpCallSubquery *op) {
    ASSERT(op->records != NULL);

    if(!op->is_returning) {
        // if there is a record to return from the input records, return it
        // NOTICE: The order of records reverses here.
        return array_len(op->records) > 0 ? array_pop(op->records) : NULL;
    }

    // returning subquery
    // TBD
    else {
        // just for compilation
        Record r;
        return r;
    }
}

// eagerly consume and aggregate all the records from the lhs (if exists). pass
// the aggregated record-list to the ArgumentList operation.
// after aggregating, return the records one-by-one to the parent op
// merges the records if is_returning is on.
static Record CallSubqueryConsumeEager
(
    OpBase *opBase  // operation
) {
    OpCallSubquery *op = (OpCallSubquery *)opBase;

    // if eager consumption has already occurred, don't consume again
    if(!op->first) {
        return _handoff_eager(op);
    }

    // make sure next entries don't get here
    op->first = false;

    op->records = array_new(Record, 1);

    // eagerly consume all records from lhs if exists or create a
    // dummy-record, and place them op->records
    Record r;
    if(op->lhs) {
        while((r = OpBase_Consume(op->lhs))) {
            array_append(op->records, r);
        }
    } else {
        r = OpBase_CreateRecord(op->body);
        array_append(op->records, r);
    }

    // plant a clone of the list of records in the ArgumentList op
    // this needs to be a clone because if the subquery is a unit subquery,
    // we want to return the records we got in the first place, with no change
    ArgumentList_AddRecordList(op->argument_list, op->records);

    // if the subquery is non-returning, consume and free all records from body
    if(!op->is_returning) {
        while((r = OpBase_Consume(op->body))) {
            OpBase_DeleteRecord(r);
        }
    }
    // eager returning subquery case: TBD.

    return _handoff_eager(op);
}

// tries to consume a record from body. if successful, return the merged\unmerged
// record with the input record (op->r) according to op->is_returning.
// if unsuccessful, returns NULL
static Record _handoff(OpCallSubquery *op) {
    ASSERT(op->r != NULL);

    // if returning subquery: consume --> merge --> return merged.
    // if unit subquery: consume until depleted, and return the current record

    Record consumed;
    if(op->is_returning) {
        consumed = OpBase_Consume(op->body);
        if(consumed == NULL) {
            return NULL;
        }

        Record clone = OpBase_DeepCloneRecord(op->r);
        Record_Merge(clone, consumed);
        OpBase_DeleteRecord(consumed);
        return clone;
    }

    // unit subquery
    // drain the body, deleting (freeing) the records
    while((consumed = OpBase_Consume(op->body))) {
        OpBase_DeleteRecord(consumed);
    }
    return op->r;
}

// similar consume to the Apply op, differing from it in that a lhs is optional,
// and that it merges the records if is_returning is on.
// responsibility for the records remains within the op.
static Record CallSubqueryConsume
(
    OpBase *opBase  // operation
) {
    OpCallSubquery *op = (OpCallSubquery *)opBase;

    // if there are more records to consume from body, consume them before
    // consuming another record from lhs
    if(op->r) {
        Record ret = _handoff(op);
        if(ret != NULL) {
            return ret;
        }
        // body has depleted from the current record (multiple records may be
        // generated due to one input record), consume from lhs again if exists
        // otherwise, return NULL
        if(op->lhs) {
            op->r = OpBase_Consume(op->lhs);
        } else {
            // // reset children ops (needed?)
            // OpBase_PropagateReset(op);
            return NULL;
        }

        // depleted
        if(op->r == NULL) {
            return NULL;
        }

        // plant a clone of the record consumed at the Argument op
        Argument_AddRecord(op->argument, OpBase_DeepCloneRecord(op->r));

        return _handoff(op);
    }

    // consume from lhs if exists, otherwise create dummy-record to pass to rhs
    // (the latter case will happen AT MOST once)
    if(op->lhs) {
        op->r = OpBase_Consume(op->lhs);
    } else {
        op->r = OpBase_CreateRecord(op->body);
    }

    // plant the record consumed at the Argument op
    Argument_AddRecord(op->argument, OpBase_DeepCloneRecord(op->r));

    // _handoff will return AT LEAST one record (since called directly after
    // calling consume on lhs), so this is safe
    return _handoff(op);

}

// // free CallSubquery internal data structures
// static void _freeInternals
// (
// 	OpCallSubquery *op  // operation to free
// ) {
//     // TBD
// }

static OpResult CallSubqueryReset
(
    OpBase *opBase  // operation
) {
    OpCallSubquery *op = (OpCallSubquery *)opBase;

    // non-eager case (RO)
    if(op->r) {
        OpBase_DeleteRecord(op->r);
        op->r = NULL;
    }

    // eager case
    else if(op->records && (op->records) > 0) {
	    op->first = true;
        array_free_cb(op->records, OpBase_DeleteRecord);
        op->records = NULL;
    }

	return OP_OK;
}

static OpBase *CallSubqueryClone
(
    const ExecutionPlan *plan,  // plan
    const OpBase *opBase        // operation to clone
) {
    ASSERT(opBase->type == OPType_CallSubquery);
	OpCallSubquery *op = (OpCallSubquery *) opBase;

	return NewCallSubqueryOp(plan, op->is_eager, op->is_returning);
}

static void CallSubqueryFree
(
	OpBase *op
) {
	OpCallSubquery *_op = (OpCallSubquery *) op;

	// TBD
}
