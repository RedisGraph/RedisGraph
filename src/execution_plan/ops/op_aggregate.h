/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __OP_AGGREGATE_H__
#define __OP_AGGREGATE_H__

#include "op.h"
#include "../../redismodule.h"
#include "../../resultset/resultset.h"
#include "../../graph/query_graph.h"
#include "../../grouping/group_cache.h"
#include "../../arithmetic/arithmetic_expression.h"

// Matrix, vector operations.
typedef enum {
    AGGREGATED,
    NON_AGGREGATED,
} ExpClassification;

/* Aggregate
 * aggregates graph according to  
 * return clause */
 typedef struct {
    OpBase op;
    AST *ast;
    AR_ExpNode **exps;
    AR_ExpNode **order_exps;
    AR_ExpNode **non_aggregated_expressions;       /* Array of arithmetic expression. */
    ExpClassification *expression_classification;  /* classifies expression as aggregated/non-aggregated. */
    Group *group;                                  /* Last accessed group. */
    TrieMap *groups;
    SIValue *group_keys;                           /* Array of values composing an aggregated group. */
    CacheGroupIterator *groupIter;
 } OpAggregate;

OpBase* NewAggregateOp(AR_ExpNode **expressions, uint *modifies);
OpResult AggregateInit(OpBase *opBase);
Record AggregateConsume(OpBase *opBase);
OpResult AggregateReset(OpBase *opBase);
void AggregateFree(OpBase *opBase);

#endif
