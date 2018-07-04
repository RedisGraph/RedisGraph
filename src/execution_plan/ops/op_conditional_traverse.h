#ifndef __OP_COND_TRAVERSE_H
#define __OP_COND_TRAVERSE_H

#include "op.h"
#include "../../rmutil/sds.h"
#include "../../parser/ast.h"
#include "../../arithmetic/algebraic_expression.h"
#include "../../arithmetic/tuples_iter.h"
#include "../../rmutil/vector.h"

/* CondTraverseStates 
 * Different states in which CondTraverse can be at. */
typedef enum {
    CondTraverseUninitialized, /* CondTraverse wasn't initialized it. */
    CondTraverseResetted,      /* CondTraverse was just restarted. */
    CondTraverseConsuming,     /* CondTraverse consuming data. */
} CondTraverseStates;

/* OP Traverse */
typedef struct {
    OpBase op;
    Graph *graph;
    AlgebraicExpression *algebraic_expression;
    AlgebraicExpressionResult *algebraic_results;
    GrB_Vector V;
    GrB_Matrix M;
    TuplesIter *iter;
    CondTraverseStates state;
} CondTraverse;

/* Creates a new Traverse operation */
OpBase* NewCondTraverseOp(Graph *g, QueryGraph* qg, AlgebraicExpression *algebraic_expression);
CondTraverse* NewCondTraverse(Graph *g, QueryGraph* qg, AlgebraicExpression *algebraic_expression);

/* TraverseConsume next operation 
 * each call will update the graph
 * returns OP_DEPLETED when no additional updates are available */
OpResult CondTraverseConsume(OpBase *opBase, QueryGraph* graph);

/* Restart iterator */
OpResult CondTraverseReset(OpBase *ctx);

/* Frees Traverse*/
void CondTraverseFree(OpBase *ctx);

#endif