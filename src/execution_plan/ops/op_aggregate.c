#include "op_aggregate.h"
#include "../../aggregate/aggregate.h"
#include "../../rmutil/strings.h"
#include "../../grouping/group.h"
#include "../../grouping/group_cache.h"
#include "../../query_executor.h"

OpBase* NewAggregateOp(RedisModuleCtx *ctx, QueryExpressionNode *ast) {
    return NewAggregate(ctx, ast);
}

Aggregate* NewAggregate(RedisModuleCtx *ctx, QueryExpressionNode *ast) {
    Aggregate *aggregate = malloc(sizeof(Aggregate));
    aggregate->ctx = ctx;
    aggregate->ast = ast;
    aggregate->refreshAfterPass = 0;
    aggregate->init = 0;

    aggregate->op.name = "Aggregate";
    aggregate->op.next = AggregateConsume;
    aggregate->op.reset = AggregateReset;
    aggregate->op.free = AggregateFree;
    return aggregate;
}

void _aggregateRecord(RedisModuleCtx *ctx, const ReturnNode* returnTree, const Graph* g) {
    // Get group
    Vector* groupKeys = ReturnClause_RetrieveGroupKeys(ctx, returnTree, g);
    char* groupKey = NULL;
    RMUtil_StringConcat(groupKeys, ",", &groupKey);
    // TODO: free groupKeys

    Group* group = NULL;
    CacheGroupGet(groupKey, &group);

    if(group == NULL) {
        // Create a new group
        // Get aggregation functions
        group = NewGroup(groupKeys, ReturnClause_GetAggFuncs(ctx, returnTree));
        CacheGroupAdd(groupKey, group);
    }

    // TODO: why can't we free groupKey?
    // free(groupKey);

    Vector* valsToAgg = ReturnClause_RetrieveGroupAggVals(ctx, returnTree, g);

    // Run each value through its coresponding function.
    for(int i = 0; i < Vector_Size(valsToAgg); i++) {
        RedisModuleString* value = NULL;
        Vector_Get(valsToAgg, i, &value);
        size_t len;
        const char* strValue = RedisModule_StringPtrLen(value, &len);

        // Convert to double SIValue.
        SIValue numValue;
        numValue.type = T_DOUBLE;
        SI_ParseValue(&numValue, strValue, len);

        AggCtx* funcCtx = NULL;
        Vector_Get(group->aggregationFunctions, i, &funcCtx);
        Agg_Step(funcCtx, &numValue, 1);
    }
    // TODO: free valsToAgg
}

OpResult AggregateConsume(OpBase *opBase, Graph* graph) {
    Aggregate* op = opBase;

    if(!op->init) {
        op->init = 1;
        return OP_DEPLETED;
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

void AggregateFree(Aggregate *opBase) {
    Aggregate *op = opBase;
    free(op);
}