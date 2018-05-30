#include "op_traverse.h"
#include "../../util/arr.h"

OpBase* NewTraverseOp(Graph *graph, AlgebraicExpression *algebraic_expression) {
    return (OpBase*)NewTraverse(graph, algebraic_expression);
}

Traverse* NewTraverse(Graph *graph, AlgebraicExpression *algebraic_expression) {
    Traverse *traverse = calloc(1, sizeof(Traverse));
    traverse->graph = graph;
    traverse->algebraic_results = AlgebraicExpression_Execute(algebraic_expression);

    GrB_Matrix M = traverse->algebraic_results->m;
    traverse->it = TuplesIter_new(M);

    // Set our Op operations
    traverse->op.name = "Traverse";
    traverse->op.type = OPType_TRAVERSE;
    traverse->op.consume = TraverseConsume;
    traverse->op.reset = TraverseReset;
    traverse->op.free = TraverseFree;

    return traverse;
}

/* TraverseConsume next operation 
 * each call will update the graph
 * returns OP_DEPLETED when no additional updates are available */
OpResult TraverseConsume(OpBase *opBase, QueryGraph* graph) {
    Traverse *op = (Traverse*)opBase;
    GrB_Index src_id;
    GrB_Index dest_id;

    if (TuplesIter_next(op->it, &src_id, &dest_id) == TuplesIter_DEPLETED) return OP_DEPLETED;

    Node *src_node = Graph_GetNode(op->graph, src_id);
    Node *dest_node = Graph_GetNode(op->graph, dest_id);
    src_node->id = src_id;
    dest_node->id = dest_id;

    *op->algebraic_results->src_node = src_node;
    *op->algebraic_results->dest_node = dest_node;

    return OP_OK;
}

OpResult TraverseReset(OpBase *ctx) {
    Traverse *op = (Traverse*)ctx;
    TuplesIter_reset(op->it);
    return OP_OK;
}

/* Frees Traverse */
void TraverseFree(OpBase *ctx) {
    Traverse *op = (Traverse*)ctx;
    TuplesIter_free(op->it);
    AlgebraicExpressionResult_Free(op->algebraic_results);
    free(op);
}
