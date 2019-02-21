/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_aggregate.h"
#include "../../util/arr.h"
#include "../../util/rmalloc.h"
#include "../../grouping/group.h"
#include "../../query_executor.h"
#include "../../arithmetic/aggregate.h"

/* Construct an expression trees for both aggregated and none aggregated expressions. */
static void _build_expressions(Aggregate *op) {
    ExpandCollapsedNodes(op->ast);
    ResultSet_CreateHeader(op->resultset);

    AST_ReturnNode *return_node = op->ast->returnNode;
    uint expCount = array_len(return_node->returnElements);
    
    op->none_aggregated_expressions = array_new(AR_ExpNode*, 1);
    op->expression_classification = rm_malloc(sizeof(uint8_t) * expCount);

    // Compose RETURN clause expressions.
    for(uint i = 0; i < expCount; i++) {
        AST_ReturnElementNode *returnElement = return_node->returnElements[i];
        
        AR_ExpNode *exp = AR_EXP_BuildFromAST(op->ast, returnElement->exp);
        if(!AR_EXP_ContainsAggregation(exp, NULL)) {
            op->expression_classification[i] = 0;
            op->none_aggregated_expressions = array_append(op->none_aggregated_expressions, exp);
        } else {
            op->expression_classification[i] = 1;
            AR_EXP_Free(exp);
        }
    }

    // ORDER-BY expressions.
    AST_OrderNode *order_node = op->ast->orderNode;
    if(order_node) {
        expCount = array_len(order_node->expressions);
        op->order_expressions = rm_malloc(sizeof(AR_ExpNode*) * expCount);
        for(uint i = 0; i < expCount; i++) {
            op->order_expressions[i] = AR_EXP_BuildFromAST(op->ast, order_node->expressions[i]);
        }
    }
}

static AR_ExpNode** _build_aggregated_expressions(Aggregate *op) {
    AST_ReturnNode *return_node = op->ast->returnNode;
    AR_ExpNode **agg_exps = array_new(AR_ExpNode*, 1);
    uint exp_count = array_len(return_node->returnElements);

    for(uint i = 0; i < exp_count; i++) {
        if(!op->expression_classification[i]) continue;
        AST_ReturnElementNode *returnElement = return_node->returnElements[i];
        AR_ExpNode *exp = AR_EXP_BuildFromAST(op->ast, returnElement->exp);
        agg_exps = array_append(agg_exps, exp);
    }

    return agg_exps;
}

static Group* _CreateGroup(Aggregate *op, Record r) {
    /* Create a new group
     * Get a fresh copy of aggregation functions. */
    AR_ExpNode **agg_exps = _build_aggregated_expressions(op);

    /* Clone group keys. */
    uint32_t key_count = array_len(op->none_aggregated_expressions);
    SIValue *group_keys = rm_malloc(sizeof(SIValue) * key_count);
    for(uint32_t i = 0; i < key_count; i++) group_keys[i] = op->group_keys[i];

    // There's no need to keep a reference to record
    // if we're not performing aggregations.
    if(!op->ast->orderNode) r = NULL;
    op->group = NewGroup(key_count, group_keys, agg_exps, r);

    return op->group;
}

/* Retrieves group under which given record belongs to,
 * creates group if one doesn't exists. */
static Group* _GetGroup(Aggregate *op, Record r) {
    // GroupBy without none-aggregated fields.
    if(!op->group_keys) {
        if(!op->group) {
            op->group = _CreateGroup(op, r);
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
        op->group = _CreateGroup(op, r);
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
    int returnElemCount = array_len(op->ast->returnNode->returnElements);
    int orderElemCount = (op->ast->orderNode) ? array_len(op->ast->orderNode->expressions) : 0;
    Record r = Record_New(returnElemCount + orderElemCount);

    // Populate record.
    int aggIdx = 0; // Index into group aggregated expressions.
    int keyIdx = 0; // Index into group keys.
    
    for(int i = 0; i < returnElemCount; i++) {
        SIValue res;
        if(op->expression_classification[i]) {
            // Aggregated expression, get aggregated value.
            AR_ExpNode *exp = group->aggregationFunctions[aggIdx++];
            AR_EXP_Reduce(exp);
            res = AR_EXP_Evaluate(exp, NULL);
            Record_AddScalar(r, i, res);
        } else {
            // None aggregated expression.
            res = group->keys[keyIdx++];
            Record_AddScalar(r, i, res);
        }

        /* TODO: this entire block can be improved, performancewise.
         * assuming the number of groups is relative small, 
         * this might be negligible. */
        if(op->ast->orderNode) {
            // If expression is aliased, introduce it to group record
            // for later evaluation by ORDER-BY expressions.
            char *alias = op->ast->returnNode->returnElements[i]->alias;
            if(alias) {
                int recIdx = AST_GetAliasID(op->ast, alias);
                Record_AddScalar(group->r, recIdx, res);
            }
        }
    }

    // Tack on order by expressions, for SORT operation to process.
    for(int i = 0; i < orderElemCount; i++) {
        SIValue res = AR_EXP_Evaluate(op->order_expressions[i], group->r);
        Record_AddScalar(r, returnElemCount+i, res);
    }

    return r;
}

OpBase* NewAggregateOp(ResultSet *resultset) {
    Aggregate *aggregate = malloc(sizeof(Aggregate));
    aggregate->init = 0;
    aggregate->ast = AST_GetFromLTS();
    aggregate->resultset = resultset;
    aggregate->none_aggregated_expressions = NULL;
    aggregate->expression_classification = NULL;
    aggregate->order_expressions = NULL;
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

    if(op->order_expressions) {
        uint32_t expCount = array_len(op->ast->orderNode->expressions);
        for(int i = 0; i < expCount; i++) AR_EXP_Free(op->order_expressions[i]);
        rm_free(op->order_expressions);
    }

    FreeGroupCache(op->groups);
}
