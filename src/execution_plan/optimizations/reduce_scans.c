/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./reduce_scans.h"
#include "../ops/op_traverse.h"
#include "../ops/op_conditional_traverse.h"
#include <assert.h>

void _reduceScans(ExecutionPlanSegment *plan, OpBase *op) {
    if(op == NULL) return;
    
    // Search for consecutive traverse and scan operations.
    if(op->type == OPType_CONDITIONAL_TRAVERSE) {
        assert(op->childCount == 1);        
        OpBase *child = op->children[0];
        if(child->type == OPType_ALL_NODE_SCAN || child->type == OPType_NODE_BY_LABEL_SCAN) {
            CondTraverse *condTraversal = (CondTraverse *)op;
            
            // Consecutive traverse scan operations, no filters.
            // Replace Conditional Traverse operation with Traverse.
            OpBase *traverse = NewTraverseOp(condTraversal->graph, condTraversal->ae);
            ExecutionPlanSegment_ReplaceOp(plan, (OpBase*)condTraversal, (OpBase*)traverse);
            OpBase_Free((OpBase*)condTraversal);
            return;
        }
    }

    for(int i = 0; i < op->childCount; i++) {
        _reduceScans(plan, op->children[i]);
    }
}

void reduceScans(ExecutionPlanSegment *plan) {
    /* Do not try to remove scan operations 
     * if query is limited, in such cases keeping scan operations
     * will speed up query processing times, as we'll be able to 
     * return as early as possible. */
    
    // TODO: whenever we decide to re-enable this optimization
    // get limit from AST.
    
    // if(ResultSet_Limited(plan->result_set))
    //     return;

    // _reduceScans(plan, plan->root);
}
