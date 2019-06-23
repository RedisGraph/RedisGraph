/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "reduce_edge_count.h"
#include "../ops/ops.h"
#include "../../util/arr.h"

/* Checks if execution plan solely performs node count */
static int _identifyPattern(OpBase *root, OpResult **opResult, OpAggregate **opAggregate, OpBase **opTraverse, OpBase **opScan, int* edgeType)
{
    // Reset.
    *edgeType = GRAPH_NO_RELATION;
    *opScan = NULL;
    *opTraverse = NULL;
    *opResult = NULL;
    *opAggregate = NULL;

    OpBase *op = root;
    // Op Results.
    if (op->type != OPType_RESULTS || op->childCount != 1)
        return 0;

    *opResult = (OpResult *)op;
    op = op->children[0];

    // Op Aggregate.
    if (op->type != OPType_AGGREGATE || op->childCount != 1)
        return 0;

    // Expecting a single aggregation, without ordering.
    *opAggregate = (OpAggregate *)op;
    if ((*opAggregate)->exp_count != 1 || (*opAggregate)->order_exp_count != 0)
        return 0;

    AR_ExpNode *exp = (*opAggregate)->expressions[0];

    // Make sure aggregation performs counting.
    if (exp->type != AR_EXP_OP ||
        exp->op.type != AR_OP_AGGREGATE ||
        strcasecmp(exp->op.func_name, "count"))
        return 0;

    // Make sure Count acts on an alias.
    if (exp->op.child_count != 1)
        return 0;
    AR_ExpNode *arg = exp->op.children[0];
    if (arg->type != AR_EXP_OPERAND ||
        arg->operand.type != AR_EXP_VARIADIC ||
        arg->operand.variadic.entity_prop != NULL)
        return 0;

    op = op->children[0];
    if (op->type!= OPType_CONDITIONAL_TRAVERSE || op->childCount != 1) return 0;
    else {
        CondTraverse *condTraverse = (CondTraverse *)op;
        if (condTraverse->edgeRelationCount != 1)
            return 0;
        else{
            *edgeType = condTraverse->edgeRelationTypes[0];
        }
    }
    *opTraverse = op;

    op = op->children[0];

    // Full node scan.
    if (op->type != OPType_ALL_NODE_SCAN ||
        op->childCount != 0)
    {
        return 0;
    }
    *opScan = op;

    return 1;
}

void reduceEdgeCount(ExecutionPlan *plan, AST *ast)
{
    /* We'll only modify execution plan if it is structured as follows:
     * "Full Scan -> Conditional Traverse -> Aggregate -> Results" */
    int edgeType;
    OpBase *opScan;
    OpBase *opTraverse;
    OpResult *opResult;
    OpAggregate *opAggregate;

    /* See if execution-plan matches the pattern:
     * "Full Scan -> Conditional Traverse -> Aggregate -> Results".
     * if that's not the case, simply return without making any modifications. */
    if (!_identifyPattern(plan->root, &opResult, &opAggregate, &opTraverse, &opScan, &edgeType))
        return;

    /* User is trying to get total number of nodes in the graph
     * optimize by skiping SCAN and AGGREGATE. */
    SIValue edgeCount;
    GraphContext *gc = GraphContext_GetFromTLS();

    // If label is specified, count only labeled entities.
    if (edgeType != GRAPH_NO_RELATION)
    {
        return;
        // TODO : implement
    }
    else
    {
        edgeCount = SI_LongVal(Graph_EdgeCount(gc->g));
    }

    // Incase alias is specified: RETURN count(r) as X
    char **aliases = NULL;
    if (opAggregate->aliases)
    {
        assert(array_len(opAggregate->aliases) == 1);
        aliases = array_new(char *, 1);
        aliases = array_append(aliases, opAggregate->aliases[0]);
    }

    /* Construct a constant expression, used by a new
     * projection operation. */
    AR_ExpNode *exp = AR_EXP_NewConstOperandNode(edgeCount);
    AR_ExpNode **exps = array_new(AR_ExpNode *, 1);
    exps = array_append(exps, exp);

    OpBase *opProject = NewProjectOp(ast, exps, aliases);

    // New execution plan: "Project -> Results"
    ExecutionPlan_RemoveOp(plan, (OpBase *)opScan);
    OpBase_Free(opScan);

    ExecutionPlan_RemoveOp(plan, (OpBase *)opTraverse);
    OpBase_Free(opTraverse);

    ExecutionPlan_RemoveOp(plan, (OpBase *)opAggregate);
    OpBase_Free((OpBase *)opAggregate);

    ExecutionPlan_AddOp((OpBase *)opResult, opProject);
}