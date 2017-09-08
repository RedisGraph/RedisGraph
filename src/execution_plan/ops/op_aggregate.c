#include "op_aggregate.h"
#include "../../aggregate/aggregate.h"
#include "../../rmutil/strings.h"
#include "../../grouping/group.h"
#include "../../grouping/group_cache.h"
#include "../../query_executor.h"

OpBase* NewAggregateOp(RedisModuleCtx *ctx, AST_QueryExpressionNode *ast) {
    return (OpBase*)NewAggregate(ctx, ast);
}

Aggregate* NewAggregate(RedisModuleCtx *ctx, AST_QueryExpressionNode *ast) {
    Aggregate *aggregate = malloc(sizeof(Aggregate));
    aggregate->ctx = ctx;
    aggregate->ast = ast;
    aggregate->refreshAfterPass = 0;
    aggregate->init = 0;

    aggregate->op.name = "Aggregate";
    aggregate->op.type = OPType_AGGREGATE;
    aggregate->op.consume = AggregateConsume;
    aggregate->op.reset = AggregateReset;
    aggregate->op.free = AggregateFree;
    aggregate->op.modifies = NULL;
    return aggregate;
}

void _aggregateRecord(RedisModuleCtx *ctx, const AST_ReturnNode* returnTree, const Graph* g) {
    /* Get group */
    Vector* groupKeys = ReturnClause_RetrieveGroupKeys(returnTree, g);
    char* groupKey;
    SIValue_StringConcat(groupKeys, &groupKey);
    // TODO: free groupKeys

    Group* group = NULL;
    CacheGroupGet(groupKey, &group);

    if(!group) {
        /* Create a new group
         * Get aggregation functions. */
        group = NewGroup(groupKeys, ReturnClause_GetAggFuncs(ctx, returnTree));
        CacheGroupAdd(groupKey, group);
    }

    Vector* valsToAgg = ReturnClause_RetrieveGroupAggVals(returnTree, g);

    // Run each value through its coresponding function.
    for(int i = 0; i < Vector_Size(valsToAgg); i++) {
        SIValue *value;
        Vector_Get(valsToAgg, i, &value);
        
        AggCtx* funcCtx;
        Vector_Get(group->aggregationFunctions, i, &funcCtx);
        Agg_Step(funcCtx, value, 1);
    }
}

OpResult AggregateConsume(OpBase *opBase, Graph* graph) {
    Aggregate *op = (Aggregate*)opBase;

    if(!op->init) {
        op->init = 1;
        return OP_REFRESH;
    }

    if(op->refreshAfterPass == 1) {
        op->refreshAfterPass = 0;
        return OP_REFRESH;
    }

    _aggregateRecord(op->ctx, op->ast->returnNode, graph);
    op->refreshAfterPass = 1;
    
    return OP_OK;
}

OpResult AggregateReset(OpBase *opBase) {
    return OP_OK;
}

void AggregateFree(OpBase *opBase) {
    Aggregate *op = (Aggregate*)opBase;
    free(op);
}