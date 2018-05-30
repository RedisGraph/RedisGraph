#ifndef __OP_TRAVERSE_H
#define __OP_TRAVERSE_H

#include "op.h"
#include "../../rmutil/sds.h"
#include "../../parser/ast.h"
#include "../../arithmetic/algebraic_expression.h"
#include "../../rmutil/vector.h"
#include "../../arithmetic/tuples_iter.h"

/* OP Traverse */
typedef struct {
    OpBase op;
    Graph *graph;
    AlgebraicExpressionResult *algebraic_results;
    TuplesIter *it;
} Traverse;

/* Creates a new Traverse operation */
OpBase* NewTraverseOp(Graph *graph, AlgebraicExpression *algebraic_expression);
Traverse* NewTraverse(Graph *graph, AlgebraicExpression *algebraic_expression);

/* TraverseConsume next operation 
 * each call will update the graph
 * returns OP_DEPLETED when no additional updates are available */
OpResult TraverseConsume(OpBase *opBase, QueryGraph* graph);

/* Restart iterator */
OpResult TraverseReset(OpBase *ctx);

/* Frees Traverse*/
void TraverseFree(OpBase *ctx);

#endif