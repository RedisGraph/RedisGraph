/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "op_argument_list.h"

// The Foreach operation aggregates records from its supplier if it exists,
// which are passed in a list to the ArgumentList operation in its
// first-embedded execution plan child operation. If there is no supplier,
// Foreach creates a dummy empty record passed to the ArgumentList operation
// in order to kick-start the execution-plan execution process.
// The Argument-list operation is used to pass a list of records to the embedded
// operations representing the clauses in the FOREACH clause, one-by-one.
// a short example:
//      FOREACH(i in [1, 2, 3, 4] | CREATE (n:N {v: i}))
// will create 4 nodes, with v property values from 1 to 4 appropriately

typedef struct {
	OpBase op;

    bool first;                   // is this the first call to consume
    OpBase *body;                 // the first op in the embedded execution-plan
    Record *records;              // records aggregated by the operation
    OpBase *supplier;             // op from which records are pulled (optional)
    ArgumentList *argument_list;  // argument operation (tap)

} OpForeach;

// creates a new Foreach operation
OpBase *NewForeachOp
(
    const ExecutionPlan *plan  // execution plan
);
