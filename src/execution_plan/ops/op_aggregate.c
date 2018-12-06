/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "op_aggregate.h"
#include "../../util/arr.h"
#include "../../util/rmalloc.h"
#include "../../grouping/group.h"
#include "../../query_executor.h"
#include "../../arithmetic/aggregate.h"

/* Construct an expression trees for both aggregated and none aggregated expressions. */
static void _build_expressions(Aggregate *op) {
    AST_ReturnNode *return_node = op->ast->returnNode;

    uint aggregatedIdx = 0;
    uint noneAggregatedIdx = 0;
    uint expCount = Vector_Size(return_node->returnElements);
    
    op->none_aggregated_expressions = array_new(AR_ExpNode*, 1);
    op->expression_classification = rm_malloc(sizeof(int) * expCount);

    for(uint i = 0; i < expCount; i++) {
        AST_ReturnElementNode *returnElement;
        Vector_Get(return_node->returnElements, i, &returnElement);
        
        AR_ExpNode *exp = AR_EXP_BuildFromAST(op->ast, returnElement->exp);
        if(!AR_EXP_ContainsAggregation(exp, NULL)) {
            op->expression_classification[i] = 0;
            op->none_aggregated_expressions = array_append(op->none_aggregated_expressions, exp);
        } else {
            op->expression_classification[i] = 1;
        }
    }
}

static AR_ExpNode** _build_aggregated_expressions(Aggregate *op) {
    AST_ReturnNode *return_node = op->ast->returnNode;
    AR_ExpNode **agg_exps = array_new(AR_ExpNode *, 1);
    uint exp_count = Vector_Size(return_node->returnElements);

    for(uint i = 0; i < exp_count; i++) {
        if(!op->expression_classification[i]) continue;
        AST_ReturnElementNode *returnElement;
        Vector_Get(return_node->returnElements, i, &returnElement);
        AR_ExpNode *exp = AR_EXP_BuildFromAST(op->ast, returnElement);
        agg_exps = array_append(agg_exps, exp);
    }

    return agg_exps;
}

static Group* _CreateGroup(Aggregate *op) {
    /* Create a new group
     * Get a fresh copy of aggregation functions. */
    AR_ExpNode **agg_exps = _build_aggregated_expressions(op);

    /* Clone group keys. */
    size_t key_count = array_len(op->none_aggregated_expressions);
    SIValue *group_keys = rm_malloc(sizeof(SIValue) * key_count);
    for(int i = 0; i < key_count; i++) group_keys[i] = op->group_keys[i];

    op->group = NewGroup(key_count, group_keys, agg_exps);
    return op->group;
}

/* Construct group key based on none aggregated terms. 
 * Returns group name which must be freed by caller. */
static Group* _GetGroup(Aggregate *op, Record r) {
    // GroupBy without none-aggregated fields.
    if(!op->group_keys) {
        if(!op->group) {
            op->group = _CreateGroup(op);
            CacheGroupAdd(op->groups, "SINGLE_GROUP", op->group);
        }
        return op->group;
    }

    // GroupBy with none-aggregated fields.
    // Evaluate none-aggregated fields, see if they match
    // the last accessed group.
    bool reuseLastAccessedGroup = true;
    uint32_t expCount = array_len(op->none_aggregated_expressions);
    for(int i = 0; i < expCount; i++) {
        AR_ExpNode *exp = op->none_aggregated_expressions[i];
        op->group_keys[i] = AR_EXP_Evaluate(exp, r);
        if( reuseLastAccessedGroup &&
            op->group &&
            SIValue_Compare(op->group->keys[i], op->group_keys[i]) == 0 ) {
            reuseLastAccessedGroup = true;
        } else {
            reuseLastAccessedGroup = false;
        }
    }

    // See if we can reuse last accessed group.
    if(reuseLastAccessedGroup) return op->group;

    // Can't reuse last accessed group, lookup group by
    // its identifier key.
    char *group_key;

    // Determine required size for group key string representation.
    size_t group_len_key = SIValue_StringConcatLen(op->group_keys, expCount);
    // TODO: should be fine, due to JMalloc memory pools
    // maybe we should have a reusable buffer.
    group_key = rm_malloc(sizeof(char) * group_len_key);
    SIValue_StringConcat(op->group_keys, expCount, group_key, group_len_key);

    op->group = CacheGroupGet(op->groups, group_key);
    if(!op->group) {
        op->group = _CreateGroup(op);
        CacheGroupAdd(op->groups, group_key, op->group);
    }
    rm_free(group_key);
    return op->group;
}

static void _aggregateRecord(Aggregate *op, Record r) {
    /* Get group */
    Group* group = _GetGroup(op, r);
    assert(group);

    // Aggregate group expressions.
    uint32_t aggFuncCount = array_len(group->aggregationFunctions);
    for(int i = 0; i < aggFuncCount; i++) {
        AR_ExpNode *exp = group->aggregationFunctions[i];
        AR_EXP_Aggregate(exp, r);
    }
}

/* Returns a record populated with group data. */
static Record _handoff(Aggregate *op) {
    char *key;
    Group *group;
    if(!op->groupIter) return NULL;
    if(!CacheGroupIterNext(op->groupIter, &key, &group)) return NULL;

    // New record with len |return elements|
    int returnElemCount = Vector_Size(op->ast->returnNode->returnElements);
    Record r = Record_New(returnElemCount);

    // Populate record.
    int aggIdx = 0; // Index into group aggregated expressions.
    int keyIdx = 0; // Index into group keys.
    
    for(int i = 0; i < returnElemCount; i++) {
        if(op->expression_classification[i]) {
            // Aggregated expression, get aggregation value.
            AR_ExpNode *exp = group->aggregationFunctions[aggIdx++];
            AR_EXP_Reduce(exp);
            SIValue res = AR_EXP_Evaluate(exp, NULL);
            Record_AddScalar(r, i, res);
        } else {
            // None aggregated expression.
            Record_AddScalar(r, i, group->keys[keyIdx++]);
        }
    }

    return r;
}

OpBase* NewAggregateOp(AST *ast) {
    Aggregate *aggregate = malloc(sizeof(Aggregate));
    aggregate->ast = ast;
    aggregate->init = 0;
    aggregate->none_aggregated_expressions = NULL;
    aggregate->expression_classification = NULL;
    aggregate->group_keys = NULL;
    aggregate->groups = CacheGroupNew();
    aggregate->groupIter = NULL;
    aggregate->group = NULL;

    OpBase_Init(&aggregate->op);
    aggregate->op.name = "Aggregate";
    aggregate->op.type = OPType_AGGREGATE;
    aggregate->op.consume = AggregateConsume;
    aggregate->op.reset = AggregateReset;
    aggregate->op.free = AggregateFree;

    return (OpBase*)aggregate;
}

Record AggregateConsume(OpBase *opBase) {
    Aggregate *op = (Aggregate*)opBase;
    OpBase *child = op->op.children[0];

    if(!op->init) {
        _build_expressions(op);
        /* Allocate memory for group keys. */
        uint32_t noneAggExpCount = array_len(op->none_aggregated_expressions);
        if(noneAggExpCount) op->group_keys = rm_malloc(sizeof(SIValue) * noneAggExpCount);
        op->init = 1;
    }

    if(op->groupIter) return _handoff(op);

    Record r;
    while((r = child->consume(child))) {
        _aggregateRecord(op, r);
        Record_Free(r);
    }

    op->groupIter = CacheGroupIter(op->groups);

    return _handoff(op);
}

OpResult AggregateReset(OpBase *opBase) {
    Aggregate *op = (Aggregate*)opBase;

    FreeGroupCache(op->groups);
    op->groups = CacheGroupNew();

    if(op->groupIter) {
        CacheGroupIterator_Free(op->groupIter);
        op->groupIter = NULL;
    }

    return OP_OK;
}

void AggregateFree(OpBase *opBase) {
    Aggregate *op = (Aggregate*)opBase;
    if(!op) return;

    if(op->group_keys) rm_free(op->group_keys);
    if(op->groupIter) CacheGroupIterator_Free(op->groupIter);
    if(op->expression_classification) rm_free(op->expression_classification);

    if(op->none_aggregated_expressions) {
        uint32_t expCount = array_len(op->none_aggregated_expressions);
        for(int i = 0; i < expCount; i++) AR_EXP_Free(op->none_aggregated_expressions[i]);
        array_free(op->none_aggregated_expressions);
    }
    FreeGroupCache(op->groups);
}
