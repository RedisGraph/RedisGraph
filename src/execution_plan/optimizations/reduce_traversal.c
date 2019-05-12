/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "reduce_traversal.h"

#include "../../util/arr.h"
#include "../../util/vector.h"
#include "../ops/op_expand_into.h"
#include "../ops/op_conditional_traverse.h"

static bool _entity_resolved(OpBase *root, const char *entity) {
    int count = (root->modifies) ? Vector_Size(root->modifies) : 0;

    for(int i = 0; i < count; i++) {
        char *alias;
        Vector_Get(root->modifies, i, &alias);
        if(strcmp(entity, alias) == 0) return true;
    }

    for(int i = 0; i < root->childCount; i++) {
        OpBase *child = root->children[i];
        if(_entity_resolved(child, entity)) return true;
    }

    return false;
}

/* Inspect each traverse operation T,
 * For each T see if T's source and destination nodes
 * are already resolved, in which case replace traversal operation 
 * with expand-into op. */
void reduceTraversal(ExecutionPlan *plan) {
    // OPType t = OPType_CONDITIONAL_TRAVERSE | OPType_CONDITIONAL_VAR_LEN_TRAVERSE;
    OPType t = OPType_CONDITIONAL_TRAVERSE;
    OpBase **traversals = ExecutionPlan_LocateOps(plan->root, t);
    uint traversals_count = array_len(traversals);
    for(uint i = 0; i < traversals_count; i++) {
        CondTraverse *traverse = (CondTraverse*)traversals[i];

        /* If traverse src and dest nodes are the same,
         * traverse acts as a filter which make sure the node is of a specific type
         * e.g. MATCH (a:A)-[e*]->(b:B) RETURN e 
         * in this case there will be a traverse operation which will 
         * filter our dest nodes (b) which aren't of type B.
         * TODO: maybe we should use the Apply operation here (not sure need to verify)
         * in this case we will not reduce to expand into. */
        if(traverse->algebraic_expression->src_node == traverse->algebraic_expression->dest_node)
            continue;

        /* Search to see if dest is already resolved,
         * TODO: Explain why src is guaranteed to be resolved. */
        const char *dest = traverse->algebraic_expression->dest_node->alias;
        printf("Trying to reduce traverse with src: %s and dest: %s\n", traverse->algebraic_expression->src_node->alias, traverse->algebraic_expression->dest_node->alias);
        if(_entity_resolved(traverse->op.children[0], dest)) {
            /* Both src and dest are already known
             * perform expand into instaed of traverse. */
            OpBase *expand_into = NewExpandIntoOp(traverse->algebraic_expression,
                                                  traverse->srcNodeRecIdx,
                                                  traverse->destNodeRecIdx,
                                                  traverse->edgeRecIdx);

            /* Set traverse algebraic_expression to NULL to avoid
             * early free. */
            traverse->algebraic_expression = NULL;
            ExecutionPlan_ReplaceOp(plan, (OpBase*)traverse, expand_into);
            OpBase_Free((OpBase*)traverse);
        }
    }

    // Clean up.
    array_free(traversals);
}
