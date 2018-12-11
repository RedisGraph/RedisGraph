/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "op_aggregate.h"
#include "../../arithmetic/aggregate.h"
#include "../../grouping/group.h"
#include "../../grouping/group_cache.h"
#include "../../query_executor.h"

OpBase* NewAggregateOp(AST *ast, TrieMap *groups) {
    Aggregate *aggregate = malloc(sizeof(Aggregate));
    aggregate->ast = ast;
    aggregate->init = 0;
    aggregate->none_aggregated_expression_count = 0;
    aggregate->none_aggregated_expressions = NULL;
    aggregate->group_keys = NULL;
    aggregate->groups = groups;

    OpBase_Init(&aggregate->op);
    aggregate->op.name = "Aggregate";
    aggregate->op.type = OPType_AGGREGATE;
    aggregate->op.consume = AggregateConsume;
    aggregate->op.reset = AggregateReset;
    aggregate->op.free = AggregateFree;

    return (OpBase*)aggregate;
}

/* Construct an aggregated expression tree foreach aggregated term. 
 * Returns a vector of aggregated expression trees. */
Vector* _build_aggregated_expressions(AST *ast) {
    Vector *aggregated_expressions = NewVector(AR_ExpNode*, 1);

    for(int i = 0; i < Vector_Size(ast->returnNode->returnElements); i++) {
        AST_ReturnElementNode *returnElement;
        Vector_Get(ast->returnNode->returnElements, i, &returnElement);

        AR_ExpNode *expression = AR_EXP_BuildFromAST(ast, returnElement->exp);
        if(AR_EXP_ContainsAggregation(expression, NULL)) {
            Vector_Push(aggregated_expressions, expression);
        }
    }

    return aggregated_expressions;
}

/* Construct group key based on none aggregated terms. 
 * Returns group name which must be freed by caller. */
char* _computeGroupKey(Aggregate *op, SIValue *group_keys, Record r) {
    if(!group_keys) return "SINGLE_GROUP";
    char *str_group;

    for(int i = 0; i < op->none_aggregated_expression_count; i++) {
        AR_ExpNode *exp = op->none_aggregated_expressions[i];
        group_keys[i] = AR_EXP_Evaluate(exp, r);
    }

    // Determine required size for group string representation.
    size_t str_group_len = SIValue_StringConcatLen(group_keys,op->none_aggregated_expression_count);
    str_group = malloc(sizeof(char) * str_group_len);

    SIValue_StringConcat(group_keys, op->none_aggregated_expression_count, str_group, str_group_len);
    return str_group;
}

void _aggregateRecord(Aggregate *op, Record r) {
    /* Get group */
    Group* group = NULL;
    char *group_key = _computeGroupKey(op, op->group_keys, r);
    CacheGroupGet(op->groups, group_key, &group);

    if(!group) {
        /* Create a new group
         * Get aggregation functions. */
        Vector *agg_exps = _build_aggregated_expressions(op->ast);

        /* Clone group keys. */
        size_t key_count = op->none_aggregated_expression_count;
        SIValue *group_keys = malloc(sizeof(SIValue) * op->none_aggregated_expression_count);
        for(int i = 0; i < op->none_aggregated_expression_count; i++) {
            group_keys[i] = op->group_keys[i];
        }

        group = NewGroup(key_count, group_keys, agg_exps);
        CacheGroupAdd(op->groups, group_key, group);
    }
    
    // Aggregate group expressions.
    for(int i = 0; i < Vector_Size(group->aggregationFunctions); i++) {
        AR_ExpNode *exp;
        Vector_Get(group->aggregationFunctions, i, &exp);
        AR_EXP_Aggregate(exp, r);
    }
}

OpResult AggregateConsume(OpBase *opBase, Record r) {
    Aggregate *op = (Aggregate*)opBase;
    OpBase *child = op->op.children[0];

    if(!op->init) {
        Build_None_Aggregated_Arithmetic_Expressions(op->ast,
                                                     &op->none_aggregated_expressions,
                                                     &op->none_aggregated_expression_count);
        /* Allocate memory for group keys. */
        if(op->none_aggregated_expression_count > 0) {
            op->group_keys = malloc(sizeof(SIValue) * op->none_aggregated_expression_count);
        }
        op->init = 1;
    }

    OpResult res = child->consume(child, r);
    if(res != OP_OK) return res;

    _aggregateRecord(op, r);

    return OP_OK;
}

OpResult AggregateReset(OpBase *opBase) {
    return OP_OK;
}

void AggregateFree(OpBase *opBase) {
    Aggregate *op = (Aggregate*)opBase;
    if(op->none_aggregated_expression_count) {
        for(int i = 0; i < op->none_aggregated_expression_count; i++) {
            AR_EXP_Free(op->none_aggregated_expressions[i]);
        }
        free(op->none_aggregated_expressions);
    }
}
