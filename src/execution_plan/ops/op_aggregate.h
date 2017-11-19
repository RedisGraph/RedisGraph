#ifndef __OP_AGGREGATE_H__
#define __OP_AGGREGATE_H__

#include "op.h"
#include "../../parser/ast.h"
#include "../../redismodule.h"
#include "../../graph/graph.h"
#include "../../arithmetic_expression.h"

/* Aggregate
 * aggregates graph according to  
 * return clause */
 typedef struct {
     OpBase op;
     RedisModuleCtx *ctx;
     int refreshAfterPass;
     AST_QueryExpressionNode *ast;
     int none_aggregated_expression_count; /* Number of return terms which are not aggregated. */
     AR_ExpNode **none_aggregated_expressions;
     SIValue *group_keys;   /* Array of values composing an aggregated group. */
     int init;
 } Aggregate;

OpBase* NewAggregateOp(RedisModuleCtx *ctx, AST_QueryExpressionNode *ast);
Aggregate* NewAggregate(RedisModuleCtx *ctx, AST_QueryExpressionNode *ast);
OpResult AggregateConsume(OpBase *opBase, Graph* graph);
OpResult AggregateReset(OpBase *opBase);
void AggregateFree(OpBase *opBase);

#endif