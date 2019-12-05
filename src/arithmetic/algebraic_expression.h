/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef ALGEBRAIC_EXPRESSION_H
#define ALGEBRAIC_EXPRESSION_H

#include "../graph/query_graph.h"
#include "../graph/graph.h"

// Matrix, vector operations.
typedef enum {
	AL_EXP_UNARY,
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
		GrB_Matrix operand;
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

AlgebraicExpressionNode *AlgebraicExpressionNode_NewOperationNode(AL_EXP_OP op);
AlgebraicExpressionNode *AlgebraicExpressionNode_NewOperandNode(GrB_Matrix operand);
// AppendLeftChild and AppendRightChild are only used in unit tests.
void AlgebraicExpressionNode_AppendLeftChild(AlgebraicExpressionNode *root,
											 AlgebraicExpressionNode *child);
void AlgebraicExpressionNode_AppendRightChild(AlgebraicExpressionNode *root,
											  AlgebraicExpressionNode *child);
void AlgebraicExpression_SumOfMul(AlgebraicExpressionNode **root);
void AlgebraicExpression_Eval(AlgebraicExpressionNode *exp, GrB_Matrix res);

/* AlgebraicExpressionOperand a single operand within an
 * algebraic expression. */
typedef struct {
	bool diagonal;          // Diagonal matrix.
	bool transpose;         // Should the matrix be transposed.
	bool free;              // Should the matrix be freed?
	GrB_Matrix operand;
} AlgebraicExpressionOperand;

// Algebraic expression e.g. A*B*C
typedef struct {
	AL_EXP_OP op;                           // Operation to perform.
	size_t operand_count;                   // Number of operands.
	size_t operand_cap;                     // Allocated number of operands.
	AlgebraicExpressionOperand *operands;   // Array of operands.
	QGNode *src_node;                       // Nodes represented by the first operand columns.
	QGNode *dest_node;                      // Nodes represented by the last operand rows.
	QGEdge *qg_edge;                        // Edge represented by sole operand.
	const char *src;                        // Alias of source node.
	const char *dest;                       // Alias of destination node.
	const char *edge;                       // Alias of sole edge operand, if one is present.
} AlgebraicExpression;

/* Constructs an empty expression. */
AlgebraicExpression *AlgebraicExpression_Empty(void);

/* Construct algebraic expression(s) from query graph. */
AlgebraicExpression **AlgebraicExpression_FromQueryGraph(
	const QueryGraph *g,    // Graph to construct expression from.
	uint *exp_count         // Number of expressions created.
);

/* Executes given expression. */
void AlgebraicExpression_Execute(AlgebraicExpression *ae, GrB_Matrix res);

/* Appends m as the last term in the expression ae. */
void AlgebraicExpression_AppendTerm(AlgebraicExpression *ae, GrB_Matrix m, bool transposeOp,
									bool freeOp, bool diagonal);

/* Prepend m as the first term in the expression ae. */
void AlgebraicExpression_PrependTerm(AlgebraicExpression *ae, GrB_Matrix m, bool transposeOp,
									 bool freeOp, bool diagonal);

/* Removes operand at position idx */
void AlgebraicExpression_RemoveTerm(AlgebraicExpression *ae, int idx,
									AlgebraicExpressionOperand *operand);

/* Whenever we decide to transpose an expression, call this function
 * directly accessing expression transpose flag is forbidden. */
void AlgebraicExpression_Transpose(AlgebraicExpression *ae);

// Set node and edge pointers in an AlgebraicExpression to reference the given QueryGraph.
void AlgebraicExpression_BindGraphEntities(const QueryGraph *qg, AlgebraicExpression *ae);

void AlgebraicExpression_Free(AlgebraicExpression *ae);

#endif

