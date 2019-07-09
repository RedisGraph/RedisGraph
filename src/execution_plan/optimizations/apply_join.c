/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "apply_join.h"
#include "../../util/arr.h"
#include "../ops/op_filter.h"
#include "../../../deps/rax/rax.h"
#include "../ops/op_value_hash_join.h"
#include "../ops/op_cartesian_product.h"

// Tests to see if given filter can act as a join condition.
static bool _applicableFilter(const FT_FilterNode *f) {
    return (f->t == FT_N_PRED && f->pred.op == EQ);
}

// Collects all consecutive filters beneath given op.
static Filter** _locate_filters(OpBase *cp) {
    OpBase *parent = cp->parent;
    Filter **filters = array_new(Filter*, 0);

    while(parent && parent->type == OPType_FILTER) {
        filters = array_append(filters, (Filter*)parent);
        parent = parent->parent;
    }

    return filters;
}

// Tests if stream resolves all aliases.
static bool _stream_resolves_entities(const OpBase *root, rax *aliases) {
    if(root->modifies) {
        for(int i = 0; i < Vector_Size(root->modifies); i++) {
            char *alias;
            Vector_Get(root->modifies, i, &alias);            
            raxRemove(aliases, (unsigned char*)alias, strlen(alias), NULL);
        }
    }

    if(raxSize(aliases) == 0) return true;

    for(int i = 0; i < root->childCount; i++) {
        if(_stream_resolves_entities(root->children[i], aliases)) {
            return true;
        }
    }

    return false;
}

/* filter is composed of two expressions:
 * left and right hand side
 * to apply a join operation each expression
 * must be entirely resolved by one of the branches
 * either left or right of the cartesian product operation
 * _relate_exp_to_stream will try to associate each expression
 * lhs and rhs with the appropriate branch. */
static void _relate_exp_to_stream(const OpBase *cp, const FT_FilterNode *f, AR_ExpNode **lhs, AR_ExpNode **rhs) {
    *lhs = NULL;
    *rhs = NULL;
    bool expression_resolved = true;

    AR_ExpNode *lhs_exp = f->pred.lhs;
    AR_ExpNode *rhs_exp = f->pred.rhs;

    assert(cp->childCount == 2);
    OpBase *left_child = cp->children[0];
    OpBase *right_child = cp->children[1];

    rax *aliases = raxNew();

    /* Make sure LHS and RHS expressions can be resolved.
     * Left expression - Left branch
     * Right expression - Right branch */
    AR_EXP_CollectAliases(lhs_exp, aliases);    
    if(_stream_resolves_entities(left_child, aliases)) {
        // aliases is now empty.
        AR_EXP_CollectAliases(rhs_exp, aliases);        
        if(_stream_resolves_entities(right_child, aliases)) {            
            *lhs = lhs_exp;
            *rhs = rhs_exp;
            raxFree(aliases);
            return;
        }
    }

    /* Left expression - Right branch
     * Right expression - Left branch */
    *lhs = NULL;
    *rhs = NULL;

    raxFree(aliases);
    aliases = raxNew();

    AR_EXP_CollectAliases(lhs_exp, aliases);
    if(_stream_resolves_entities(right_child, aliases)) {
        // aliases is now empty.
        AR_EXP_CollectAliases(rhs_exp, aliases);
        if(_stream_resolves_entities(left_child, aliases)) {
            *lhs = rhs_exp;
            *rhs = lhs_exp;
            raxFree(aliases);
            return;
        }
    }

    // Couldn't completely resolve either lhs_exp or rhs_exp.
    *lhs = NULL;
    *rhs = NULL;
    raxFree(aliases);
}

/* Try to replace cartesian product with a value hash join operation */
void applyJoin(ExecutionPlan *plan) {
    OpBase **cps = ExecutionPlan_LocateOps(plan->root, OPType_CARTESIAN_PRODUCT);
    int cp_count = array_len(cps);

    for(int i = 0; i < cp_count; i++) {
        OpBase *cp = cps[i];
        /* TODO: change the way our cartesian product
         * today we alow for cartesian product with > 2 child ops
         * I think it might be better to create a chain of cartesian products
         * where each pulls from exactly 2 streams
         * consider: MATCH a,b,c WHERE a.v = b.v. */
        if(cp->childCount != 2) continue;
        Filter **filters = _locate_filters(cp);

        int filter_count = array_len(filters);
        for(int j = 0; j < filter_count; j++) {
            Filter *filter = filters[j];
            if(_applicableFilter(filter->filterTree)) {
                // Reduce cartesian product to value hash join
                AR_ExpNode *lhs = NULL;
                AR_ExpNode *rhs = NULL;

                /* Make sure lhs expression is resolved by  
                 * left stream, if not swap. */
                _relate_exp_to_stream(cp, filter->filterTree, &lhs, &rhs);
                /* There are cases where either lhs or rhs expressions
                 * require data from both streams, consider:
                 * a.v + c.v = b.v + d.v 
                 * where a and d are resolved by lhs stream and 
                 * b and c are resolved by rhs stream. 
                 * in which case we can't perform join. */
                if(lhs == NULL || rhs == NULL) continue;

                assert(lhs != rhs);
                OpBase *value_hash_join = NewValueHashJoin(lhs, rhs);

                /* Remove filter which is now part of the join operation
                 * replace cartesian product with join. */
                ExecutionPlan_RemoveOp(plan, (OpBase*)filter);
                ExecutionPlan_ReplaceOp(plan, cp, value_hash_join);
                break;
            }
        }
        array_free(filters);
    }
    array_free(cps);
}
