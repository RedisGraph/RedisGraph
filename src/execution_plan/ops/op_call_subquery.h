/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "op_argument.h"
#include "op_argument_list.h"

// The Call-Subquery operation is similar to the Apply operation, differing
// from it in two main aspects:
//  1. It eagerly consumes the records of its first child (lhs) IF the body of
//     the subquery contains a consuming (eager) operation (depicted in the
//     is_eager field), and plants that record-list in the ArgumentList op.
//     Otherwise, it will non-eagerly consume, and plant in the Argument op.
//  2. It merges the records of the rhs branch and the record consumed from lhs
//     according to the `is_returning` field.
//  TODO:
//  TBD: It will probably also be different in the responsibility (it won't have)
//       over its records (record list in this case).

typedef struct {
	OpBase op;

    bool first;                   // is this the first call to consume
    bool is_eager;                // is the op eager
    bool is_returning;            // is the subquery returning or unit
    OpBase *body;                 // the first op in the embedded execution-plan
    OpBase *lhs;                  // op from which records are pulled (optional)
    Record r;                     // current record consumed from lhs
    Record *records;              // records aggregated by the operation
    Argument *argument;           // Argument operation (potential tap)
    ArgumentList *argument_list;  // ArgumentList operation (potential tap)

} OpCallSubquery;

// creates a new CallSubquery operation
OpBase *NewCallSubqueryOp
(
	const ExecutionPlan *plan,  // execution plan
    bool is_eager,              // if an updating clause lies in the body, eagerly consume the records
    bool is_returning           // is the subquery returning or unit
);
