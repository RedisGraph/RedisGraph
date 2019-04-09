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

/* Initialize expression_classification[i] = AGGREGATED if expressions[i] 
 * is an aggregated expression, NONE_AGGREGATED otherwise,
 * In addition keeps track after none aggregated expression within
 * a seperated array.*/
static void _classify_expressions(OpAggregate *op) {
    uint expCount = array_len(op->expressions);
    op->none_aggregated_expressions = array_new(AR_ExpNode*, 0);
    op->expression_classification = rm_malloc(sizeof(ExpClassification) * expCount);

    for(uint i = 0; i < expCount; i++) {
        AR_ExpNode *exp = op->expressions[i];
        if(!AR_EXP_ContainsAggregation(exp, NULL)) {
            op->expression_classification[i] = NONE_AGGREGATED;
            op->none_aggregated_expressions = array_append(op->none_aggregated_expressions, exp);
        } else {
            op->expression_classification[i] = AGGREGATED;
        }
    }
}

static AR_ExpNode** _build_aggregated_expressions(OpAggregate *op) {
    AR_ExpNode **agg_exps = array_new(AR_ExpNode*, 1);
    
    for(uint i = 0; i < array_len(op->expressions); i++) {
        if(op->expression_classification[i] == NONE_AGGREGATED) continue;
        AR_ExpNode *exp = AR_EXP_DuplicateAggFunc(op->ast->return_expressions[i]->exp);
        agg_exps = array_append(agg_exps, exp);
    }

    return agg_exps;
}

static Group* _CreateGroup(OpAggregate *op, Record r) {
    /* Create a new group
     * Get a fresh copy of aggregation functions. */
    AR_ExpNode **agg_exps = _build_aggregated_expressions(op);

    /* Clone group keys. */
    uint key_count = array_len(op->none_aggregated_expressions);
    SIValue *group_keys = rm_malloc(sizeof(SIValue) * key_count);
    for(uint i = 0; i < key_count; i++) group_keys[i] = op->group_keys[i];

    /* There's no need to keep a reference to record if we're not sorting groups. */
    if(!op->order_exps) op->group = NewGroup(key_count, group_keys, agg_exps, NULL);
    else op->group = NewGroup(key_count, group_keys, agg_exps, r);

    return op->group;
}

static void _ComputeGroupKey(OpAggregate *op, Record r) {
    uint expCount = array_len(op->none_aggregated_expressions);

    for(uint i = 0; i < expCount; i++) {
        AR_ExpNode *exp = op->none_aggregated_expressions[i];
        op->group_keys[i] = AR_EXP_Evaluate(exp, r);
    }
}

static void _ComputeGroupKeyStr(OpAggregate *op, char **key) {
    uint none_agg_exp_count = array_len(op->none_aggregated_expressions);
    if(none_agg_exp_count == 0) {
        *key = rm_strdup("SINGLE_GROUP");
        return;
    }

    // Determine required size for group key string representation.
    size_t key_len = SIValue_StringConcatLen(op->group_keys, none_agg_exp_count);
    *key = rm_malloc(sizeof(char) * key_len);
    SIValue_StringConcat(op->group_keys, none_agg_exp_count, *key, key_len);
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

    // Evaluate none-aggregated fields, see if they match
    // the last accessed group.
    bool reuseLastAccessedGroup = true;
    uint expCount = array_len(op->none_aggregated_expressions);
    for(uint i = 0; i < expCount; i++) {
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

    // Aggregate group expressions.
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

    Record r = Record_New(op->exp_count + op->order_exp_count);

    // Populate record.
    uint aggIdx = 0; // Index into group aggregated expressions.
    uint keyIdx = 0; // Index into group keys.
    
    for(uint i = 0; i < op->exp_count; i++) {
        SIValue res;
        if(op->expression_classification[i] == AGGREGATED) {
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
        if(op->order_exps) {
            // If expression is aliased, introduce it to group record
            // for later evaluation by ORDER-BY expressions.
            const char *alias = op->ast->return_expressions[i]->alias;
            if(alias) Record_AddScalar(group->r, NEWAST_GetAliasID(op->ast, (char*)alias), res);
        }
    }

    // Tack order by expressions for SORT operation to process.
    for(unsigned short i = 0; i < op->order_exp_count; i++) {
        SIValue res = AR_EXP_Evaluate(op->order_exps[i], group->r);
        Record_AddScalar(r, op->exp_count+i, res);
    }

    return r;
}

OpBase* NewAggregateOp(AR_ExpNode **expressions, char **aliases) {
    OpAggregate *aggregate = malloc(sizeof(OpAggregate));
    NEWAST *ast = NEWAST_GetFromTLS();
    aggregate->ast = ast;
    aggregate->aliases = aliases;
    aggregate->expressions = expressions;
    aggregate->exp_count = array_len(expressions);
    aggregate->expression_classification = NULL;
    aggregate->none_aggregated_expressions = NULL;
    aggregate->order_exps = NULL;
    aggregate->order_exp_count = 0;
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
    
    aggregate->op.modifies = NewVector(char*, 0);
    for(uint i = 0; i < array_len(aliases); i++) {
        if(aliases[i]) Vector_Push(aggregate->op.modifies, aliases[i]);
    }

    return (OpBase*)aggregate;
}

OpResult AggregateInit(OpBase *opBase) {
    OpAggregate *op = (OpAggregate*)opBase;
    _classify_expressions(op);
    AR_ExpNode **order_exps = _getOrderExpressions(opBase->parent);
    if (order_exps) {
        op->order_exps = order_exps;
        op->order_exp_count = array_len(order_exps);
    }

    /* Allocate memory for group keys. */
    uint noneAggExpCount = array_len(op->none_aggregated_expressions);
    if(noneAggExpCount) op->group_keys = rm_malloc(sizeof(SIValue) * noneAggExpCount);
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
    if(op->none_aggregated_expressions) array_free(op->none_aggregated_expressions);

    if(op->expressions) {
        uint expCount = array_len(op->expressions);
        for(uint i = 0; i < expCount; i++) AR_EXP_Free(op->expressions[i]);
        array_free(op->expressions);
    }
    array_free(op->aliases);

    FreeGroupCache(op->groups);
}
