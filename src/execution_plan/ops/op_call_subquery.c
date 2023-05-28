/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "op_join.h"
#include "op_call_subquery.h"

// forward declarations
static void CallSubqueryFree(OpBase *opBase);
static OpResult CallSubqueryInit(OpBase *opBase);
static Record CallSubqueryConsume(OpBase *opBase);
static OpResult CallSubqueryReset(OpBase *opBase);
static Record CallSubqueryConsumeEager(OpBase *opBase);
static OpBase *CallSubqueryClone(const ExecutionPlan *plan, const OpBase *opBase);

// find the deepest child of a root operation, and append it to the
// arguments/arguemnt_lists array of the CallSubquery operation
static void _find_set_deepest
(
    OpCallSubquery *call_subquery,  // CallSubquery operation
    OpBase *root                    // root op
) {
    while(root->childCount > 0) {
        root = root->children[0];
    }
    if(call_subquery->is_eager) {
        ASSERT(OpBase_Type((const OpBase *)root) == OPType_ARGUMENT_LIST);
        array_append(call_subquery->argument_lists, (ArgumentList *)root);
    } else {
        ASSERT(OpBase_Type((const OpBase *)root) == OPType_ARGUMENT);
        array_append(call_subquery->arguments, (Argument *)root);
    }
}

// creates a new CallSubquery operation
OpBase *NewCallSubqueryOp
(
	const ExecutionPlan *plan,  // execution plan
    bool is_eager,              // if an updating clause lies in the body, eagerly consume the records
    bool is_returning           // is the subquery returning or unit
) {
    OpCallSubquery *op = rm_calloc(1, sizeof(OpCallSubquery));

    op->r              = NULL;
    op->lhs            = NULL;
    op->body           = NULL;
    op->first          = true;
    op->records        = NULL;
    op->arguments      = NULL;
    op->argument_lists = NULL;
    op->is_eager       = is_eager;
    op->is_returning   = is_returning;

    // set the consume function according to eagerness of the op
    Record (*consumeFunc)(OpBase *opBase) = is_eager ?
        CallSubqueryConsumeEager :
        CallSubqueryConsume;

    OpBase_Init((OpBase *)op, OPType_CallSubquery, "CallSubquery",
        CallSubqueryInit, consumeFunc, CallSubqueryReset, NULL,
        CallSubqueryClone, CallSubqueryFree, false, plan);

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

    // search for the ArgumentList\Argument ops, depending if the op is eager
    // the non-relevant field will stay NULL
    if(op->is_eager) {
        op->argument_lists = array_new(ArgumentList *, 1);
    } else {
        op->arguments = array_new(Argument *, 1);
    }

    bool join = (OpBase_Type(op->body) == OPType_JOIN) ||
                (OpBase_Type(OpBase_GetChild(op->body, 0)) == OPType_JOIN);
    if(join) {
        OpJoin *join = (OpBase_Type(op->body) == OPType_JOIN) ?
                        (OpJoin *)op->body :
                        (OpJoin *)OpBase_GetChild(op->body, 0);

        uint n_branches = OpBase_ChildCount((OpBase *)join);

        for(uint i = 0; i < n_branches; i++) {
            OpBase *deepest = OpBase_GetChild((OpBase *)join, i);
            _find_set_deepest(op, deepest);
        }
    } else {
        OpBase *deepest = op->body;
        _find_set_deepest(op, deepest);
    }

    return OP_OK;
}

// passes a record to the parent op.
// if the subquery is non-returning, all the records have already been
// consumed from the body (child depleted), so that we only need to return the
// received records.
// if the subquery is returning, return a the record received from the body
static Record _handoff_eager(OpCallSubquery *op) {
    ASSERT(op->is_returning || op->records != NULL);

    if(!op->is_returning) {
        // if there is a record to return from the input records, return it
        // NOTICE: The order of records reverses here.
        return array_len(op->records) > 0 ? array_pop(op->records) : NULL;
    }

    // returning subquery
    return OpBase_Consume(op->body);
}

// eagerly consume and aggregate all the records from the lhs (if exists). pass
// the aggregated record-list to the ArgumentList operation\s.
// after aggregating, return the consumed records one-by-one to the parent op,
// if is_returning is on.
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
    // dummy-record, and place them\it in op->records
    Record r;
    if(op->lhs) {
        while((r = OpBase_Consume(op->lhs))) {
            array_append(op->records, r);
        }
        // propagate reset to lhs, to release RediSearch index locks (if any)
        OpBase_PropagateReset(op->lhs);
    } else {
        r = OpBase_CreateRecord(op->body);
        array_append(op->records, r);
    }

    int n_branches = (int)array_len(op->argument_lists);
    for(int i = 0; i < n_branches - 1; i++) {
        Record *records_clone;
        array_clone_with_cb(records_clone, op->records,
            OpBase_DeepCloneRecord);
        ArgumentList_AddRecordList(op->argument_lists[i], records_clone);
    }

    if(op->is_returning) {
        // give the last branch the original records
        ArgumentList_AddRecordList(op->argument_lists[n_branches - 1],
            op->records);

        // responsibility for the records is passed to the argumentList op
        op->records = NULL;
    } else {
        // give the last branch a clone of the original records
        Record *records_clone;
        array_clone_with_cb(records_clone, op->records,
            OpBase_DeepCloneRecord);
        ArgumentList_AddRecordList(op->argument_lists[n_branches - 1],
            records_clone);

        // consume and free all records from body
        while((r = OpBase_Consume(op->body))) {
            OpBase_DeleteRecord(r);
        }
    }

    return _handoff_eager(op);
}

// tries to consumes a record from the body, merge it with the current input
// record and return it. If body is depleted from this record, tries to consume
// a record from the lhs, and repeat the process (if the lhs record is not NULL)
static Record _consume_and_return
(
    OpCallSubquery *op
) {
    Record consumed;
    consumed = OpBase_Consume(op->body);
    if(consumed == NULL) {
        OpBase_PropagateReset(op->body);
        OpBase_DeleteRecord(op->r);
        op->r = NULL;
        if(op->lhs && (op->r = OpBase_Consume(op->lhs)) != NULL) {
            // plant a clone of the record consumed at the Argument ops
            uint n_branches = array_len(op->arguments);
            for(uint i = 0; i < n_branches; i++) {
                Argument_AddRecord(op->arguments[i], OpBase_DeepCloneRecord(op->r));
            }
        } else {
            // lhs depleted --> CALL {} depleted as well
            return NULL;
        }
        // goto return_cycle;
        return _consume_and_return(op);
    }

    Record clone = OpBase_DeepCloneRecord(op->r);
    // Merge consumed record into a clone of the received record.
    Record_Merge(clone, consumed);
    OpBase_DeleteRecord(consumed);
    return clone;
}

// tries to consume a record from the body. if successful, return the
// merged\unmerged record with the input record (op->r) according to the
// is_returning flag.
// depletes child if is_returning is off (body records not needed).
static Record _handoff(OpCallSubquery *op) {
    ASSERT(op->r != NULL);

    // returning subquery: consume --> merge --> return merged.
    if(op->is_returning) {
        return _consume_and_return(op);
    }

    Record consumed;
    // non-returning subquery: drain the body, deleting (freeing) the records
    // return current record
    while((consumed = OpBase_Consume(op->body))) {
        OpBase_DeleteRecord(consumed);
    }
    // TODO: make sure this is necessary.
    OpBase_PropagateReset(op->body);
    Record r = op->r;
    op->r = NULL;
    return r;
}

// consumes a record from the lhs, plants it in the Argument\List ops, and
// consumes a record from the body.
// if there is already a record in op->r, it means that the body has not been
// depleted from the current record yet, and we should consume from it before
// consuming another record from lhs
static Record CallSubqueryConsume
(
    OpBase *opBase  // operation
) {
    OpCallSubquery *op = (OpCallSubquery *)opBase;

    // if there are more records to consume from body, consume them before
    // consuming another record from lhs
    if(op->r) {
        return _handoff(op);
    }

    // consume from lhs if exists, otherwise create dummy-record to pass to rhs
    // (the latter case will happen AT MOST once)
    if(op->lhs) {
        op->r = OpBase_Consume(op->lhs);
    } else if(op->first){
        op->r = OpBase_CreateRecord((OpBase *)op);
        // op->r = OpBase_CreateRecord(op->body);
        op->first = false;
    }

    // plant the record consumed at the Argument ops
    if(op->r) {
        uint n_branches = array_len(op->arguments);
        for(uint i = 0; i < n_branches; i++) {
            Argument_AddRecord(op->arguments[i], OpBase_DeepCloneRecord(op->r));
        }
    } else {
        // no records - lhs depleted
        return NULL;
    }

    return _handoff(op);
}

// frees CallSubquery internal data structures
static void _freeInternals
(
	OpCallSubquery *op  // operation to free
) {
    if(op->records != NULL) {
        uint n_records = array_len(op->records);
        for(uint i = 0; i < n_records; i++) {
            OpBase_DeleteRecord(op->records[i]);
        }
        array_free(op->records);
        op->records = NULL;
    }
}

// resets a CallSubquery operation
static OpResult CallSubqueryReset
(
    OpBase *opBase  // operation
) {
    OpCallSubquery *op = (OpCallSubquery *)opBase;

    _freeInternals(op);

	return OP_OK;
}

// clones a CallSubquery operation
static OpBase *CallSubqueryClone
(
    const ExecutionPlan *plan,  // plan
    const OpBase *opBase        // operation to clone
) {
    ASSERT(opBase->type == OPType_CallSubquery);
	OpCallSubquery *op = (OpCallSubquery *) opBase;

	return NewCallSubqueryOp(plan, op->is_eager, op->is_returning);
}

// frees a CallSubquery operation
static void CallSubqueryFree
(
	OpBase *op
) {
	OpCallSubquery *_op = (OpCallSubquery *) op;

    _freeInternals(_op);

    if(_op->arguments != NULL) {
        array_free(_op->arguments);
        _op->arguments = NULL;
    }
    if(_op->argument_lists != NULL) {
        array_free(_op->argument_lists);
        _op->argument_lists = NULL;
    }
}
