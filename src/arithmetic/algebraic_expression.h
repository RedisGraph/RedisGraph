/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

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
    bool _transpose;    // Result got transposed.
} AlgebraicExpressionResult;

/* AlgebraicExpressionOperand a single operand within an
 * algebraic expression. */
typedef struct  {
    bool transpose;         // Should the matrix be transposed.
    GrB_Matrix operand;
} AlgebraicExpressionOperand;

// Algebraic expression e.g. A*B*C
typedef struct {
    AL_EXP_OP op;                           // Operation to perform.
    size_t operand_count;                   // Number of operands.
    AlgebraicExpressionOperand *operands;   // Array of operands.
    Node **src_node;                        // Nodes represented by the first operand rows.
    Node **dest_node;                       // Nodes represented by the last operand columns.
    bool _transpose;                         // Transpose result.
} AlgebraicExpression;

/* Construct an algebraic expression from a query. */
AlgebraicExpression **AlgebraicExpression_From_Query(const AST_Query *ast, Vector *matchPattern, const QueryGraph *q, size_t *exp_count);

/* Executes given expression. */
AlgebraicExpressionResult *AlgebraicExpression_Execute(AlgebraicExpression *ae);

/* Appends m as the last term in the expression ae. */
void AlgebraicExpression_AppendTerm(AlgebraicExpression *ae, GrB_Matrix m, bool transpose);

/* Prepend m as the first term in the expression ae. */
void AlgebraicExpression_PrependTerm(AlgebraicExpression *ae, GrB_Matrix m, bool transpose);

/* Whenever we decide to transpose an expression, call this function
 * directly accessing expression transpose flag is forbidden. */
void AlgebraicExpression_Transpose(AlgebraicExpression *ae);

void AlgebraicExpression_Free(AlgebraicExpression* ae);
void AlgebraicExpressionResult_Free(AlgebraicExpressionResult *aer);

#endif