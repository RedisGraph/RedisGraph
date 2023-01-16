/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "op_argument_list.h"

// The Foreach operation gets a list from either its first child or statically,
// unwinds it, and executes clauses which receive the components of the unwinded
// list as input records.
// It passes on the records it received onward to its parent operation,
// unchanged.
// The Argument-list operation is used to pass a list of records to the embedded
// operations representing the clauses in the FOREACH clause, one-by-one.

typedef struct {
	OpBase op;
    bool first;                   // is this the first call to consume
    Record *records;              // records aggregated by the operation
    OpBase *supplier;             // op from which records are pulled (optional)
    OpBase *first_embedded;       // the first op in the embedded execution-plan
    ArgumentList *argument_list;  // argument operation (tap)
} OpForeach;

// creates a new Foreach operation
OpBase *NewForeachOp
(
    const ExecutionPlan *plan  // execution plan
);

