/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "op_foreach.h"
#include "../../errors.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"
#include "../../util/rmalloc.h"
#include "../../datatypes/array.h"
#include "../../arithmetic/arithmetic_expression.h"

#define INDEX_NOT_SET UINT_MAX

/* Forward declarations. */
static OpResult ForeachInit(OpBase *opBase);
static Record ForeachConsume(OpBase *opBase);
static OpResult ForeachReset(OpBase *opBase);
static OpBase *ForeachClone(const ExecutionPlan *plan, const OpBase *opBase);

/* Creates a new Foreach operation */
OpBase *NewForeachOp
(
    const ExecutionPlan *plan  // execution plan
) {
    OpForeach *op = rm_calloc(1, sizeof(OpForeach));

    op->supplier = NULL;
	op->first_embedded = NULL;
    op->argument = NULL;
	op->first = false;

    OpBase_Init((OpBase *)op, OPType_FOREACH, "Foreach", ForeachInit, ForeachConsume,
				ForeachReset, NULL, ForeachClone, NULL, false, plan);

	return (OpBase *)op;
}

static OpResult ForeachInit
(
    OpBase *opBase  // operation
) {
    OpForeach *op = (OpForeach *)opBase;

	// if number of children is larger than one --> we have a supplier and 
	// a consumer. Otherwise, we have only a consumer.
	if(op->op.childCount > 1) {
		op->supplier = op->op.children[0];
		op->first_embedded = op->op.children[1];
	}
	else {
		op->first_embedded = op->op.children[0];
	}

	// update argument operation to be the deepest child of the first embedded
	// child
	OpBase *argument = op->first_embedded;
	while(argument->childCount > 0) {
		argument = argument->children[0];
	}
	op->argument = (Argument *)argument;

	op->first = true;

    return OP_OK;
}

static Record ForeachConsume
(
    OpBase *opBase  // operation
) {
    Record r = NULL;
    OpForeach *op = (OpForeach *)opBase;

	// consume a record from supplier if it exists
	if(op->supplier) {
		r = OpBase_Consume(op->supplier);
	}
	if(r == NULL) {
		// depleted or no child
		if(!op->first) {
			// Done
			return NULL;
		}
		// first call and no child --> call consume on first embedded ONCE
		// plan the record in the argument operation
		Argument_AddRecord(op->argument, OpBase_CreateRecord((OpBase *)op));
		op->first = false;
	}
	else {
		Argument_AddRecord(op->argument, OpBase_CloneRecord(r));
	}
	// call consume on first_embedded operation
	OpBase_Consume(op->first_embedded);

	return r;
}
// FOREACH(n in [1, 2, 3, 4] | ....)
static OpResult ForeachReset
(
    OpBase *opBase  // operation
) {
    OpForeach *op = (OpForeach *)opBase;
	op->first = false;

	return OP_OK;
}

static OpBase *ForeachClone
(
    const ExecutionPlan *plan,  // plan
    const OpBase *opBase        // operation
) {
    ASSERT(opBase->type == OPType_FOREACH);
	return NewForeachOp(plan);
}
