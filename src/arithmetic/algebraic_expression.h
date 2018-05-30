#ifndef ALGEBRAIC_EXPRESSION_H
#define ALGEBRAIC_EXPRESSION_H

#include "../graph/query_graph.h"
#include "../graph/graph.h"
#include "../parser/ast.h"

// Matrix, vector operations.
typedef enum {
    AL_EXP_ADD,
    AL_EXP_MUL,
    AL_EXP_AND,
} AL_EXP_OP;

// Result of an algebraic expression evaluation.
typedef struct {
    GrB_Matrix m;       // Resulting matrix.
    Node **src_node;    // Nodes represented by matrix rows.
    Node **dest_node;   // Nodes represented by matrix columns.
    bool _free_m;       // Should M be freed or not.
} AlgebraicExpressionResult;

// Algebraic expression e.g. A*B*C
typedef struct {
    AL_EXP_OP op;           // Operation to perform.
    size_t operand_count;   // Number of operands to operate on.
    GrB_Matrix *operands;   // Array of operands to operate on.
    Node **_src_node;       // Nodes represented by the first operand rows.
    Node **_dest_node;      // Nodes represented by the last operand columns.
} AlgebraicExpression;

/* Construct an algebraic expression from a query. */
AlgebraicExpression **AlgebraicExpression_From_Query(const AST_Query *ast, const QueryGraph *q, size_t *exp_count);

/* Executes given expression. */
AlgebraicExpressionResult *AlgebraicExpression_Execute(AlgebraicExpression *ae);

void AlgebraicExpression_Free(AlgebraicExpression* ae);
void AlgebraicExpressionResult_Free(AlgebraicExpressionResult *aer);

#endif