/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "reduce_count.h"
#include "../ops/ops.h"
#include "../../util/arr.h"

static GrB_UnaryOp countMultipleEdges = NULL;

static int _identifyResultAndAggregateOps(OpBase *root, OpResult** opResult, OpAggregate** opAggregate) {
    OpBase *op = root;
    // Op Results.
    if(op->type != OPType_RESULTS || op->childCount != 1) return 0;

    *opResult = (OpResult*)op;
    op = op->children[0];

    // Op Aggregate.
    if(op->type != OPType_AGGREGATE || op->childCount != 1) return 0;

    // Expecting a single aggregation, without ordering.
    *opAggregate = (OpAggregate*)op;
    if((*opAggregate)->exp_count != 1 || (*opAggregate)->order_exp_count != 0) return 0;

    AR_ExpNode *exp = (*opAggregate)->expressions[0];

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
    
    return 1;

}
/* Checks if execution plan solely performs node count */
static int _identifyNodeCountPattern(OpBase *root, OpResult **opResult, OpAggregate **opAggregate, OpBase **opScan, char **label) {
    // Reset.
    *label = NULL;
    *opScan = NULL;
    *opResult = NULL;
    *opAggregate = NULL;

    if(!_identifyResultAndAggregateOps(root, opResult, opAggregate)) return 0;
    OpBase* op = ((OpBase*)*opAggregate)->children[0];

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

bool _reduceNodeCount(ExecutionPlan *plan, AST *ast){
/* We'll only modify execution plan if it is structured as follows:
     * "Scan -> Aggregate -> Results" */
    char *label;
    OpBase *opScan;
    OpResult *opResult;
    OpAggregate *opAggregate;

    /* See if execution-plan matches the pattern:
     * "Scan -> Aggregate -> Results".
     * if that's not the case, simply return without making any modifications. */
    if(!_identifyNodeCountPattern(plan->root, &opResult, &opAggregate, &opScan, &label)) return false;

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
    char **aliases = NULL;
    if(opAggregate->aliases) {
        assert(array_len(opAggregate->aliases) == 1);
        aliases = array_new(char*, 1);
        aliases = array_append(aliases, opAggregate->aliases[0]);
    }

    /* Construct a constant expression, used by a new
     * projection operation. */
    AR_ExpNode *exp = AR_EXP_NewConstOperandNode(nodeCount);
    AR_ExpNode **exps = array_new(AR_ExpNode*, 1);
    exps = array_append(exps, exp);

    OpBase *opProject = NewProjectOp(ast, exps, aliases);

    // New execution plan: "Project -> Results"    
    ExecutionPlan_RemoveOp(plan, (OpBase*)opScan);    
    OpBase_Free(opScan);

    ExecutionPlan_RemoveOp(plan, (OpBase*)opAggregate);
    OpBase_Free((OpBase*)opAggregate);

    ExecutionPlan_AddOp((OpBase*)opResult, opProject);
    return true;
}

/* Checks if execution plan solely performs edge count */
static int _identifyEdgeCountPattern(OpBase *root, OpResult **opResult, OpAggregate **opAggregate, OpBase **opTraverse, OpBase **opScan) {
    // Reset.
    *opScan = NULL;
    *opTraverse = NULL;
    *opResult = NULL;
    *opAggregate = NULL;

    if(!_identifyResultAndAggregateOps(root, opResult, opAggregate)) return 0;
    OpBase* op = ((OpBase*)*opAggregate)->children[0];

    if (op->type!= OPType_CONDITIONAL_TRAVERSE || op->childCount != 1) return 0;
    *opTraverse = op;
    op = op->children[0];

    // Full node scan.
    if (op->type != OPType_ALL_NODE_SCAN || op->childCount != 0) return 0;
    *opScan = op;

    return 1;
}

void _countEdges(void *z, const void *x) {
    uint64_t *entry = (uint64_t*) x;
    if(SINGLE_EDGE(*entry)) {
        *(uint64_t*)z = 1;
    } else {
        EdgeID *ids = (EdgeID*)(*entry);
        *(uint64_t*)z = array_len(ids);
    }
}

uint64_t _countRelationshipEdges(GrB_Matrix M) {
    // Create Unary operation only once.
    if(!countMultipleEdges) {
        GrB_UnaryOp_new(&countMultipleEdges, _countEdges,
                        GrB_UINT64, GrB_UINT64);
    }

    /* TODO: to avoid this entire process keep track if
     * M contains multiple edges between two given nodes
     * if there are no multiple edges, 
     * i.e. `a` is connected to `b` with multiple edges of type R
     * then all we need to do is return M's nnz.
     * Otherwise create a new matrix A, where A[i,j] = x 
     * where x is the number of edges in M[i,j]
     * then reduce A using the plus (sum) monoid. */

    GrB_Index nrows;
    GrB_Index ncols;
    GrB_Matrix_nrows(&nrows, M);
    GrB_Matrix_ncols(&ncols, M);

    GrB_Matrix A;
    GrB_Matrix_new(&A, GrB_UINT64, nrows, ncols);

    // A[i,j] = # of edges in M[i,j].
    GrB_Matrix_apply(A, GrB_NULL, GrB_NULL,
                     countMultipleEdges, M, GrB_NULL);

    uint64_t edges = 0;
    // Sum(A)
    GrB_Matrix_reduce_UINT64(&edges, GrB_NULL,
                             GxB_PLUS_UINT64_MONOID, A, GrB_NULL);

    GrB_free(&A);
    return edges;
}

void _reduceEdgeCount(ExecutionPlan *plan, AST *ast) {
    /* We'll only modify execution plan if it is structured as follows:
     * "Full Scan -> Conditional Traverse -> Aggregate -> Results" */
    OpBase *opScan;
    OpBase *opTraverse;
    OpResult *opResult;
    OpAggregate *opAggregate;

    /* See if execution-plan matches the pattern:
     * "Full Scan -> Conditional Traverse -> Aggregate -> Results".
     * if that's not the case, simply return without making any modifications. */
    if(!_identifyEdgeCountPattern(plan->root, &opResult, &opAggregate, &opTraverse, &opScan))
        return;

    /* User is trying to get total number of edges in the graph
     * optimize by skiping SCAN, Traverse and AGGREGATE. */
    SIValue edgeCount = SI_LongVal(0);
    GraphContext *gc = GraphContext_GetFromTLS();

    // If type is specified, count only labeled entities.
    CondTraverse* condTraverse = (CondTraverse*) opTraverse;
    int edgeRelationCount = condTraverse->edgeRelationCount;
    if(condTraverse->edgeRelationTypes[0] != GRAPH_NO_RELATION) {
        uint64_t edges = 0;
        for (int i = 0; i < edgeRelationCount; i++) {
            edges += _countRelationshipEdges(Graph_GetRelationMap(gc->g, condTraverse->edgeRelationTypes[i]));
        }
        edgeCount = SI_LongVal(edges);
    } else {
        edgeCount = SI_LongVal(Graph_EdgeCount(gc->g));
    }

    // Incase alias is specified: RETURN count(r) as X
    char **aliases = NULL;
    if (opAggregate->aliases) {
        assert(array_len(opAggregate->aliases) == 1);
        aliases = array_new(char*, 1);
        aliases = array_append(aliases, opAggregate->aliases[0]);
    }

    /* Construct a constant expression, used by a new
     * projection operation. */
    AR_ExpNode *exp = AR_EXP_NewConstOperandNode(edgeCount);
    AR_ExpNode **exps = array_new(AR_ExpNode*, 1);
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

void reduceCount(ExecutionPlan *plan, AST *ast) {
    /* both _reduceNodeCount and _reduceEdgeCount should count nodes or edges, respectively, 
     * out of the same execution plan.
     * If node count optimization was unable to execute, 
     * meaning that the execution plan does not hold any node count pattern,
     * then edge count will be tried to be executed upon the same execution plan */
    bool reduceNodeCountSucsses = _reduceNodeCount(plan, ast);
    if(!reduceNodeCountSucsses) _reduceEdgeCount(plan, ast);
}
