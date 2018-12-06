/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __OP_PRODUCE_RESULTS_H
#define __OP_PRODUCE_RESULTS_H

#include "op.h"
#include "../../parser/ast.h"
#include "../../redismodule.h"
#include "../../graph/query_graph.h"
#include "../../resultset/resultset.h"

/* ProduceResults
 * generates result set */

typedef struct {
    OpBase op;
    AST *ast;
    Vector *return_elements; /* Vector of arithmetic expressions. */
    ResultSet *result_set;
} ProduceResults;


/* Creates a new NodeByLabelScan operation */
OpBase* NewProduceResultsOp(AST *ast, ResultSet *result_set, QueryGraph *graph);

/* ProduceResults next operation
 * called each time a new result record is required */
Record ProduceResultsConsume(OpBase *op);

/* Restart iterator */
OpResult ProduceResultsReset(OpBase *ctx);

/* Frees ProduceResults */
void ProduceResultsFree(OpBase *ctx);

#endif
