/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "reduce_count.h"
#include "../ops/ops.h"
#include "../../util/arr.h"

/* Checks if execution plan solely performs node count */
static int _identifyPattern(OpBase *root, OpResult **opResult, OpAggregate **opAggregate, OpBase **opScan, char **label) {
    // Reset.
    *label = NULL;
    *opScan = NULL;
    *opResult = NULL;
    *opAggregate = NULL;

    OpBase *op = root;
    // Op Results.
    if(op->type != OPType_RESULTS || op->childCount != 1) return 0;

    *opResult = (OpResult*)op;
    op = op->children[0];

    // Op Aggregate.
    if(op->type != OPType_AGGREGATE || op->childCount != 1) return 0;

    // Expecting a single aggregation, without ordering.
    *opAggregate = (OpAggregate*)op;
    uint exp_count = array_len((*opAggregate)->exps);
    uint order_exp_count = array_len((*opAggregate)->order_exps);
    if(exp_count != 1 || order_exp_count != 0) return 0;

    AR_ExpNode *exp = (*opAggregate)->exps[0];

    // Make sure aggregation performs counting.    
    if( exp->type != AR_EXP_OP ||
        exp->op.type != AR_OP_AGGREGATE ||
        strcasecmp(exp->op.func_name, "count")) return 0;

    // Make sure Count acts on an alias.
    if(exp->op.child_count != 1) return 0;
    AR_ExpNode *arg = exp->op.children[0];
    if( arg->type != AR_EXP_OPERAND ||
        arg->operand.type != AR_EXP_VARIADIC ||
        arg->operand.variadic.entity_prop != NULL) return 0;

    op = op->children[0];

    // Scan, either a full node or label scan.
    if((op->type != OPType_ALL_NODE_SCAN && 
        op->type != OPType_NODE_BY_LABEL_SCAN) ||
        op->childCount != 0) {           
           return 0;
    }

    *opScan = op;
    if(op->type == OPType_NODE_BY_LABEL_SCAN) {
        NodeByLabelScan *labelScan = (NodeByLabelScan*)op;
        assert(labelScan->node->label);
        *label = labelScan->node->label;
    }

    return 1;
}

void reduceCount(ExecutionPlan *plan) {
    /* We'll only modify execution plan if it is structured as follows:
     * "Scan -> Aggregate -> Results" */
    char *label;
    OpBase *opScan;
    OpResult *opResult;
    OpAggregate *opAggregate;

    /* See if execution-plan matches the pattern:
     * "Scan -> Aggregate -> Results".
     * if that's not the case, simply return without making any modifications. */
    if(!_identifyPattern(plan->root, &opResult, &opAggregate, &opScan, &label)) return;

    /* User is trying to get total number of nodes in the graph
     * optimize by skiping SCAN and AGGREGATE. */
    SIValue nodeCount;
    GraphContext *gc = GraphContext_GetFromTLS();

    // If label is specified, count only labeled entities.
    if(label) {
        Schema *s = GraphContext_GetSchema(gc, label, SCHEMA_NODE);
        if(s) nodeCount = SI_LongVal(Graph_LabeledNodeCount(gc->g, s->id));
        else nodeCount = SI_LongVal(0); // Specified Label doesn't exists.
    } else {
        nodeCount = SI_LongVal(Graph_NodeCount(gc->g));
    }

    // Incase alias is specified: RETURN count(n) as X
    uint *modifies = NULL;
    if(opAggregate->op.modifies) {
        assert(array_len(opAggregate->op.modifies) == 1);
        modifies = array_new(uint, 1);
        modifies = array_append(modifies, opAggregate->op.modifies[0]);
    }

    /* Construct a constant expression, used by a new
     * projection operation. */
    AR_ExpNode *exp = AR_EXP_NewConstOperandNode(nodeCount);
    AR_ExpNode **exps = array_new(AR_ExpNode*, 1);
    exps = array_append(exps, exp);

    OpBase *opProject = NewProjectOp(exps, modifies);

    // New execution plan: "Project -> Results"    
    ExecutionPlan_RemoveOp(plan, (OpBase*)opScan);    
    OpBase_Free(opScan);

    ExecutionPlan_RemoveOp(plan, (OpBase*)opAggregate);
    OpBase_Free((OpBase*)opAggregate);

    ExecutionPlan_AddOp((OpBase*)opResult, opProject);
}
