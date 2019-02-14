/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __OP_AGGREGATE_H__
#define __OP_AGGREGATE_H__

#include "op.h"
#include "../../parser/ast.h"
#include "../../redismodule.h"
#include "../../resultset/resultset.h"
#include "../../graph/query_graph.h"
#include "../../grouping/group_cache.h"
#include "../../arithmetic/arithmetic_expression.h"

/* Aggregate
 * aggregates graph according to  
 * return clause */
 typedef struct {
    OpBase op;
    AST *ast;
    ResultSet *resultset;
    AR_ExpNode **none_aggregated_expressions;   /* Array of arithmetic expression. */
    AR_ExpNode **order_expressions;             /* Array of arithmetic expression. */
    uint8_t *expression_classification;         /* 1 if RETURN_CLAUSE[i] is aggregated, 0 otherwise.  */
    SIValue *group_keys;                        /* Array of values composing an aggregated group. */
    TrieMap *groups;
    CacheGroupIterator *groupIter;
    Group *group;                               /* Last accessed group. */
    int init;
 } Aggregate;

OpBase* NewAggregateOp(ResultSet *resultset);
Record AggregateConsume(OpBase *opBase);
OpResult AggregateReset(OpBase *opBase);
void AggregateFree(OpBase *opBase);

#endif
