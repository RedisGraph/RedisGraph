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
#include "../../graph/query_graph.h"
#include "../../arithmetic/arithmetic_expression.h"

/* Aggregate
 * aggregates graph according to  
 * return clause */
 typedef struct {
     OpBase op;
     AST_Query *ast;
     int none_aggregated_expression_count; /* Number of return terms which are not aggregated. */
     AR_ExpNode **none_aggregated_expressions;
     SIValue *group_keys;   /* Array of values composing an aggregated group. */
     TrieMap *groups;
     int init;
 } Aggregate;

OpBase* NewAggregateOp(AST_Query *ast, TrieMap *groups);
OpResult AggregateConsume(OpBase *opBase, Record *r);
OpResult AggregateReset(OpBase *opBase);
void AggregateFree(OpBase *opBase);

#endif
