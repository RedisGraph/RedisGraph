#ifndef __OP_AGGREGATE_H__
#define __OP_AGGREGATE_H__

#include "op.h"
#include "../../parser/ast.h"
#include "../../redismodule.h"
#include "../../graph/graph.h"

/* Aggregate
 * aggregates graph according to  
 * return clause */
 typedef struct {
     OpBase op;
     RedisModuleCtx *ctx;
     int refreshAfterPass;
     QueryExpressionNode *ast;
     int init;  // TODO: find a better solution for this, maybe an instance of group_cache.
 } Aggregate;

OpBase* NewAggregateOp(RedisModuleCtx *ctx, QueryExpressionNode *ast);
Aggregate* NewAggregate(RedisModuleCtx *ctx, QueryExpressionNode *ast);
OpResult AggregateConsume(OpBase *opBase, Graph* graph);
OpResult AggregateReset(OpBase *opBase);
void AggregateFree(Aggregate *opBase);

#endif