/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#ifndef __OP_RESULTS_H
#define __OP_RESULTS_H

#include "op.h"
#include "../../redismodule.h"
#include "../../graph/query_graph.h"
#include "../../graph/graphcontext.h"
#include "../../resultset/resultset.h"

/* Results generates result set */

typedef struct {
	OpBase op;
	ResultSet *result_set;
} Results;


/* Creates a new NodeByLabelScan operation */
OpBase *NewResultsOp(ResultSet *result_set, QueryGraph *graph);

/* Results next operation
 * called each time a new result record is required */
Record ResultsConsume(OpBase *op);

/* Restart iterator */
OpResult ResultsReset(OpBase *ctx);

/* Frees Results */
void ResultsFree(OpBase *ctx);

#endif
