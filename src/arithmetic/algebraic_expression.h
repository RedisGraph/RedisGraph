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
    AL_EXP_TRANSPOSE,
} AL_EXP_OP;

// Type of node within an algebraic expression
typedef enum {
    AL_OPERAND,
    AL_OPERATION,
} AlgebraicExpressionNodeType;

/* Forward declarations. */
typedef struct AlgebraicExpressionNode AlgebraicExpressionNode;

struct AlgebraicExpressionNode {
    union {
        struct {
            bool entity_is_node;
            void *entity;
            GrB_Matrix mat;
        } operand;
        struct {
            AL_EXP_OP op;
            bool reusable;
            GrB_Matrix v;
            AlgebraicExpressionNode *l;
            AlgebraicExpressionNode *r;
        } operation;
    };
    AlgebraicExpressionNodeType type;
};

typedef struct {
  AlgebraicExpressionNode *exp_root;
  AlgebraicExpressionNode *exp_leaf;
  Node *src;
  Node *dest;
  Edge *edge;
} AE_Unit;

AlgebraicExpressionNode* AlgebraicExpression_InvertTree(AlgebraicExpressionNode *root);
AlgebraicExpressionNode* AlgebraicExpression_Append(AlgebraicExpressionNode *root, AlgebraicExpressionNode *child);
int AlgebraicExpression_OperandCount(AlgebraicExpressionNode *root);
AlgebraicExpressionNode* AlgebraicExpression_PopFirst(AlgebraicExpressionNode **root);
AlgebraicExpressionNode* AlgebraicExpression_PopLast(AlgebraicExpressionNode **root);
AE_Unit*** AlgebraicExpression_BuildExps(const AST *ast, const QueryGraph *q, int *component_count);

AlgebraicExpressionNode* AlgebraicExpression_NewOperationNode(AL_EXP_OP op);
AlgebraicExpressionNode* AlgebraicExpression_NewOperand(GrB_Matrix mat);
AlgebraicExpressionNode* AlgebraicExpression_NewNodeOperand(Node *operand);
AlgebraicExpressionNode* AlgebraicExpression_NewEdgeOperand(Edge *operand);
void AlgebraicExpressionNode_AppendLeftChild(AlgebraicExpressionNode *root, AlgebraicExpressionNode *child);
void AlgebraicExpressionNode_AppendRightChild(AlgebraicExpressionNode *root, AlgebraicExpressionNode *child);
void AlgebraicExpression_SumOfMul(AlgebraicExpressionNode **root);
void AlgebraicExpression_EvalWithFilter(AlgebraicExpressionNode *exp, GrB_Matrix filter, GrB_Matrix res);
void AlgebraicExpression_Eval(AlgebraicExpressionNode *exp, GrB_Matrix res);
void AlgebraicExpressionNode_Free(AlgebraicExpressionNode *root);

#endif
