/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "../../arithmetic/arithmetic_expression.h"
#include "op_argument.h"

/** Foreach operation gets (among other potentially) a list from its first child,
 * unwinds it, and executes clauses which receive the components of the list as input
 * records. 
 * It passes on the records it got, unchanged.
 * The list (potentially, among others) is pulled from the first child.
 * 
 * The Argument operation is used as a record-passer to the operations representing
 * the clauses in the FOREACH clause.
 */

typedef struct {
	OpBase op;
    
    OpBase *supplier;        // the operation from which records are pulled (optional)
    Argument *argument;      // argument operation (tap)
    OpBase *first_embedded;  // the first operation in the embedded execution-plan

    uint listIdx;            // Current list index.
    SIValue list;            // List which the unwind operation is performed on.
    AR_ExpNode *exp;         // Arithmetic expression (evaluated as an SIArray).
    int foreachRecIdx;       // Update record at this index.
    Record currentRecord;    // record to clone and add a value extracted from the list.
} OpForeach;

/* Creates a new Foreach operation */
OpBase *NewForeachOp
(
    const ExecutionPlan *plan,  // execution plan
    AR_ExpNode *exp             // arithmetic expression node
);
