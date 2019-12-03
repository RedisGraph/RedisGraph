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
	GrB_Matrix _zero_matrix;    /* Fake matrix, in-case label does not exists. */
	unsigned int nodeRecIdx;    /* Node position within record. */
	GxB_MatrixTupleIter *iter;
	Record child_record;        /* The Record this op acts on if it is not a tap. */
} NodeByLabelScan;

/* Creates a new NodeByLabelScan operation */
OpBase *NewNodeByLabelScanOp(const ExecutionPlan *plan, const QGNode *node);

