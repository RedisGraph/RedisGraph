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
    aggregate->none_aggregated_expression_count = 0;
    aggregate->none_aggregated_expressions = NULL;
    aggregate->group_keys = NULL;

    aggregate->op.name = "Aggregate";
    aggregate->op.type = OPType_AGGREGATE;
    aggregate->op.consume = AggregateConsume;
    aggregate->op.reset = AggregateReset;
    aggregate->op.free = AggregateFree;
    aggregate->op.modifies = NULL;
    return aggregate;
}

/* Construct an aggregated expression tree foreach aggregated term. 
 * Returns a vector of aggregated expression trees. */
Vector* _build_aggregated_expressions(AST_QueryExpressionNode *ast, Graph* g) {
    Vector *aggregated_expressions = NewVector(AR_ExpNode*, 1);

    for(int i = 0; i < Vector_Size(ast->returnNode->returnElements); i++) {
        AST_ReturnElementNode *returnElement;
        Vector_Get(ast->returnNode->returnElements, i, &returnElement);

        AR_ExpNode *expression = AR_EXP_BuildFromAST(returnElement->exp, g);
        if(AR_EXP_ContainsAggregation(expression, NULL)) {
            Vector_Push(aggregated_expressions, expression);
        }
    }

    return aggregated_expressions;
}

/* Construct group key based on none aggregated terms. 
 * Returns group name which must be freed by caller. */
char* _computeGroupKey(Aggregate *op, SIValue *group_keys) {
    char *group;

    for(int i = 0; i < op->none_aggregated_expression_count; i++) {
        AR_ExpNode *exp = op->none_aggregated_expressions[i];
        group_keys[i] = AR_EXP_Evaluate(exp);
    }

    SIValue_StringConcat(group_keys, op->none_aggregated_expression_count, &group);
    return group;
}

void _aggregateRecord(Aggregate *op, Graph *g) {
    /* Get group */
    Group* group = NULL;
    char *group_key = _computeGroupKey(op, op->group_keys);
    CacheGroupGet(group_key, &group);

    if(!group) {
        /* Create a new group
         * Get aggregation functions. */
        Vector *agg_exps = _build_aggregated_expressions(op->ast, g);

        /* Clone group keys. */
        size_t key_count = op->none_aggregated_expression_count;
        SIValue *group_keys = malloc(sizeof(SIValue) * op->none_aggregated_expression_count);
        for(int i = 0; i < op->none_aggregated_expression_count; i++) {
            group_keys[i] = op->group_keys[i];
        }

        group = NewGroup(key_count, group_keys, agg_exps);
        CacheGroupAdd(group_key, group);
    }
    
    // Aggregate group expressions.
    for(int i = 0; i < Vector_Size(group->aggregationFunctions); i++) {
        AR_ExpNode *exp;
        Vector_Get(group->aggregationFunctions, i, &exp);
        AR_EXP_Aggregate(exp);
    }
}

OpResult AggregateConsume(OpBase *opBase, Graph* graph) {
    Aggregate *op = (Aggregate*)opBase;

    if(!op->init) {
        Build_None_Aggregated_Arithmetic_Expressions(op->ast->returnNode,
                                                     &op->none_aggregated_expressions,
                                                     &op->none_aggregated_expression_count,
                                                     graph);
        /* Allocate memory for group keys. */
        op->group_keys = malloc(sizeof(SIValue) * op->none_aggregated_expression_count);
        op->init = 1;
        return OP_REFRESH;
    }

    if(op->refreshAfterPass == 1) {
        op->refreshAfterPass = 0;
        return OP_REFRESH;
    }

    _aggregateRecord(op, graph);
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