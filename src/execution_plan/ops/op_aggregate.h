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
     AST_QueryExpressionNode *ast;
     int init;
 } Aggregate;

OpBase* NewAggregateOp(RedisModuleCtx *ctx, AST_QueryExpressionNode *ast);
Aggregate* NewAggregate(RedisModuleCtx *ctx, AST_QueryExpressionNode *ast);
OpResult AggregateConsume(OpBase *opBase, Graph* graph);
OpResult AggregateReset(OpBase *opBase);
void AggregateFree(OpBase *opBase);

#endif