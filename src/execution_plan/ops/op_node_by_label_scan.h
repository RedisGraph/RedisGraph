/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "../../graph/graph.h"
#include "../../graph/entities/node.h"
#include "../../../deps/GraphBLAS/Include/GraphBLAS.h"

/* NodeByLabelScan, scans entire label. */

typedef struct {
	OpBase op;
	Graph *g;
	const QGNode *n;            /* Node being scanned. */
	unsigned int nodeRecIdx;    /* Node position within record. */
	NodeID currentId;       // Current ID fetched.
	NodeID minId;           // Min ID to fetch.
	NodeID maxId;           // Max ID to fetch.
	bool minInclusive;      // Include min ID.
	bool maxInclusive;      // Include max ID.
	GxB_MatrixTupleIter *iter;
	Record child_record;        /* The Record this op acts on if it is not a tap. */
} NodeByLabelScan;

/* Creates a new NodeByLabelScan operation */
OpBase *NewNodeByLabelScanOp(const ExecutionPlan *plan, const QGNode *node);

