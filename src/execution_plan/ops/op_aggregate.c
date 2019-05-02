/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_aggregate.h"
#include "op_sort.h"
#include "../../util/arr.h"
#include "../../util/rmalloc.h"
#include "../../grouping/group.h"
#include "../../query_executor.h"
#include "../../arithmetic/aggregate.h"

static AR_ExpNode** _getOrderExpressions(OpBase *op) {
    if(op == NULL) return NULL;
    // No need to look further if we haven't encountered a sort operation
    // before a project/aggregate op
    if (op->type == OPType_PROJECT || op->type == OPType_AGGREGATE) return NULL;

    if(op->type == OPType_SORT) {
        OpSort *sort = (OpSort*)op;
        return sort->expressions;
    }

    // We are only interested in a SORT operation if it is a direct parent of the aggregate op
    return NULL;
}

/* Initialize expression_classification, which denotes whether each
 * expression in the RETURN or ORDER segment is an aggregate function.
 * In addition keeps track of non-aggregated expressions in
 * a separate array.*/
static void _classify_expressions(OpAggregate *op) {
    uint exp_count = array_len(op->exps);
    uint order_exp_count = array_len(op->order_exps);
    op->non_aggregated_expressions = array_new(AR_ExpNode*, 0);
    op->expression_classification = rm_malloc(sizeof(ExpClassification) * (exp_count + order_exp_count));

    for(uint i = 0; i < exp_count; i++) {
        AR_ExpNode *exp = op->exps[i];
        if(!AR_EXP_ContainsAggregation(exp, NULL)) {
            op->expression_classification[i] = NON_AGGREGATED;
            op->non_aggregated_expressions = array_append(op->non_aggregated_expressions, exp);
        } else {
            op->expression_classification[i] = AGGREGATED;
        }
    }

    for(uint i = 0; i < order_exp_count; i++) {
        AR_ExpNode *exp = op->order_exps[i];
        if(!AR_EXP_ContainsAggregation(exp, NULL)) {
            op->expression_classification[exp_count + i] = NON_AGGREGATED;
            op->non_aggregated_expressions = array_append(op->non_aggregated_expressions, exp);
        } else {
            op->expression_classification[exp_count + i] = AGGREGATED;
        }
    }
}

static AR_ExpNode** _build_aggregated_expressions(OpAggregate *op) {
    AR_ExpNode **agg_exps = array_new(AR_ExpNode*, 1);

    uint exp_count = array_len(op->exps);
    uint order_exp_count = array_len(op->order_exps);

    for(uint i = 0; i < exp_count; i++) {
        if(op->expression_classification[i] == NON_AGGREGATED) continue;
        AR_ExpNode *exp = AR_EXP_DuplicateAggFunc(op->exps[i]);
        agg_exps = array_append(agg_exps, exp);
    }

    for(uint i = 0; i < order_exp_count; i++) {
        if(op->expression_classification[exp_count + i] == NON_AGGREGATED) continue;
        AR_ExpNode *exp = AR_EXP_DuplicateAggFunc(op->order_exps[i]);
        agg_exps = array_append(agg_exps, exp);
    }

    return agg_exps;
}

static Group* _CreateGroup(OpAggregate *op, Record r) {
    /* Create a new group
     * Get a fresh copy of aggregation functions. */
    AR_ExpNode **agg_exps = _build_aggregated_expressions(op);

    /* Clone group keys. */
    uint key_count = array_len(op->non_aggregated_expressions);
    SIValue *group_keys = rm_malloc(sizeof(SIValue) * key_count);
    for(uint i = 0; i < key_count; i++) group_keys[i] = op->group_keys[i];

    /* There's no need to keep a reference to record if we're not sorting groups. */
    if(!op->order_exps) op->group = NewGroup(key_count, group_keys, agg_exps, NULL);
    else op->group = NewGroup(key_count, group_keys, agg_exps, r);

    return op->group;
}

static void _ComputeGroupKey(OpAggregate *op, Record r) {
    uint exp_count = array_len(op->non_aggregated_expressions);

    for(uint i = 0; i < exp_count; i++) {
        AR_ExpNode *exp = op->non_aggregated_expressions[i];
        op->group_keys[i] = AR_EXP_Evaluate(exp, r);
    }
}

static void _ComputeGroupKeyStr(OpAggregate *op, char **key) {
    uint non_agg_exp_count = array_len(op->non_aggregated_expressions);
    if(non_agg_exp_count == 0) {
        *key = rm_strdup("SINGLE_GROUP");
        return;
    }

    // Determine required size for group key string representation.
    size_t key_len = SIValue_StringConcatLen(op->group_keys, non_agg_exp_count);
    *key = rm_malloc(sizeof(char) * key_len);
    SIValue_StringConcat(op->group_keys, non_agg_exp_count, *key, key_len);
}

/* Retrieves group under which given record belongs to,
 * creates group if one doesn't exists. */
static Group* _GetGroup(OpAggregate *op, Record r) {
    char *group_key_str;
    // Construct group key.
    _ComputeGroupKey(op, r);

    // First group created.
    if(!op->group) {
        op->group = _CreateGroup(op, r);
        Group_KeyStr(op->group, &group_key_str);
        CacheGroupAdd(op->groups, group_key_str, op->group);
        rm_free(group_key_str);
        return op->group;
    }

    // Evaluate non-aggregated fields, see if they match
    // the last accessed group.
    bool reuseLastAccessedGroup = true;
    uint exp_count = array_len(op->non_aggregated_expressions);
    for(uint i = 0; i < exp_count; i++) {
        if( reuseLastAccessedGroup &&
            SIValue_Compare(op->group->keys[i], op->group_keys[i]) == 0 ) {
            reuseLastAccessedGroup = true;
        } else {
            reuseLastAccessedGroup = false;
        }
    }

    // See if we can reuse last accessed group.
    if(reuseLastAccessedGroup) return op->group;

    // Can't reuse last accessed group, lookup group by identifier key.
    _ComputeGroupKeyStr(op, &group_key_str);
    op->group = CacheGroupGet(op->groups, group_key_str);
    if(op->group) return op->group;

    // Group does not exists, create it.
    op->group = _CreateGroup(op, r);
    CacheGroupAdd(op->groups, group_key_str, op->group);
    rm_free(group_key_str);
    return op->group;
}

static void _aggregateRecord(OpAggregate *op, Record r) {
    /* Get group */
    Group* group = _GetGroup(op, r);
    assert(group);

    // Aggregate group exps.
    uint aggFuncCount = array_len(group->aggregationFunctions);
    for(uint i = 0; i < aggFuncCount; i++) {
        AR_ExpNode *exp = group->aggregationFunctions[i];
        AR_EXP_Aggregate(exp, r);
    }

    /* Free record, incase it is not group representative.
     * group representative will be freed once group is freed. */
    if(group->r != r) Record_Free(r);
}

/* Returns a record populated with group data. */
static Record _handoff(OpAggregate *op) {
    char *key;
    Group *group;
    if(!op->groupIter) return NULL;
    if(!CacheGroupIterNext(op->groupIter, &key, &group)) return NULL;

    uint exp_count = array_len(op->exps);
    uint order_exp_count = array_len(op->order_exps);
    Record r = Record_New(exp_count + order_exp_count);

    // Populate record.
    uint aggIdx = 0; // Index into group aggregated exps.
    uint keyIdx = 0; // Index into group keys.
    SIValue res;

    for(uint i = 0; i < exp_count; i++) {
        if(op->expression_classification[i] == AGGREGATED) {
            // Aggregated expression, get aggregated value.
            AR_ExpNode *exp = group->aggregationFunctions[aggIdx++];
            AR_EXP_Reduce(exp);
            res = AR_EXP_Evaluate(exp, NULL);
            Record_AddScalar(r, i, res);
        } else {
            // Non-aggregated expression.
            res = group->keys[keyIdx++];
            Record_AddScalar(r, i, res);
        }
    }

    // Tack order by exps for SORT operation to process.
    for(uint i = 0; i < order_exp_count; i++) {
        if(op->expression_classification[exp_count + i] == AGGREGATED) {
            // Aggregated expression, get aggregated value.
            AR_ExpNode *exp = group->aggregationFunctions[aggIdx++];
            AR_EXP_Reduce(exp);
            res = AR_EXP_Evaluate(exp, NULL);
            Record_AddScalar(r, exp_count + i, res);
        } else {
            // Non-aggregated expression.
            res = group->keys[keyIdx++];
            Record_AddScalar(r, exp_count + i, res);
        }
    }

    return r;
}

OpBase* NewAggregateOp(AR_ExpNode **exps, uint *modifies) {
    OpAggregate *aggregate = malloc(sizeof(OpAggregate));
    AST *ast = AST_GetFromTLS();
    aggregate->ast = ast;
    aggregate->exps = exps;
    aggregate->expression_classification = NULL;
    aggregate->non_aggregated_expressions = NULL;
    aggregate->order_exps = NULL;
    aggregate->group = NULL;
    aggregate->groupIter = NULL;
    aggregate->group_keys = NULL;
    aggregate->groups = CacheGroupNew();

    OpBase_Init(&aggregate->op);
    aggregate->op.name = "Aggregate";
    aggregate->op.type = OPType_AGGREGATE;
    aggregate->op.consume = AggregateConsume;
    aggregate->op.init = AggregateInit;
    aggregate->op.reset = AggregateReset;
    aggregate->op.free = AggregateFree;

    aggregate->op.modifies = modifies;

    return (OpBase*)aggregate;
}

OpResult AggregateInit(OpBase *opBase) {
    OpAggregate *op = (OpAggregate*)opBase;
    AR_ExpNode **order_exps = _getOrderExpressions(opBase->parent);
    if (order_exps) {
        op->order_exps = order_exps;
    }

    _classify_expressions(op);

    /* Allocate memory for group keys. */
    uint nonAggExpCount = array_len(op->non_aggregated_expressions);
    if(nonAggExpCount) op->group_keys = rm_malloc(sizeof(SIValue) * nonAggExpCount);
    return OP_OK;
}

Record AggregateConsume(OpBase *opBase) {
    OpAggregate *op = (OpAggregate*)opBase;
    OpBase *child = op->op.children[0];

    if(op->groupIter) return _handoff(op);

    Record r;
    while((r = child->consume(child))) _aggregateRecord(op, r);

    op->groupIter = CacheGroupIter(op->groups);
    return _handoff(op);
}

OpResult AggregateReset(OpBase *opBase) {
    OpAggregate *op = (OpAggregate*)opBase;

    FreeGroupCache(op->groups);
    op->groups = CacheGroupNew();

    if(op->groupIter) {
        CacheGroupIterator_Free(op->groupIter);
        op->groupIter = NULL;
    }

    return OP_OK;
}

void AggregateFree(OpBase *opBase) {
    OpAggregate *op = (OpAggregate*)opBase;
    if(!op) return;

    if(op->group_keys) rm_free(op->group_keys);
    if(op->groupIter) CacheGroupIterator_Free(op->groupIter);
    if(op->expression_classification) rm_free(op->expression_classification);
    if(op->non_aggregated_expressions) array_free(op->non_aggregated_expressions);

    if(op->exps) {
        uint exp_count = array_len(op->exps);
        for(uint i = 0; i < exp_count; i++) AR_EXP_Free(op->exps[i]);
        array_free(op->exps);
    }

    FreeGroupCache(op->groups);
}
