#ifndef ALGEBRAIC_EXPRESSION_H
#define ALGEBRAIC_EXPRESSION_H

#include "../graph/query_graph.h"
#include "../graph/graph.h"

typedef struct
{

} AlgebraicExpressio;

/* Construct an algebraic expression from a query graph. */
AlgebraicExpressio* AlgebraicExpressio_From_QueryGraph(const QueryGraph *g);

/* Get a string representation of algebraic expression. */
char* AlgebraicExpression_To_String(const AlgebraicExpressio* ae);

/* Executes given expression. */
void AlgebraicExpressio_Execute(AlgebraicExpressio *ae, const Graph *g);

void AlgebraicExpressio_Free(AlgebraicExpressio* ae);

#endif