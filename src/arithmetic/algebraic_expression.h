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
            bool free;              // Should the matrix be freed?
            bool diagonal;          // Diagonal matrix.
		    GrB_Matrix matrix;      // Matrix operand.
            const char *src;        // Alias given to operand's rows (src node).
            const char *dest;       // Alias given to operand's columns (destination node).
            const char *edge;       // Alias given to operand (edge).
        } operand;
		struct {
			AL_EXP_OP op;                       // Operation: `*`,`+`,`transpose`
			AlgebraicExpressionNode **children; // Child nodes.
		} operation;
	};
	AlgebraicExpressionNodeType type;   // Type of node, either an operation or an operand.
};


//------------------------------------------------------------------------------
// AlgebraicExpression Node creation functions.
//------------------------------------------------------------------------------

// Create a new AlgebraicExpression operation node.
AlgebraicExpressionNode *AlgebraicExpressionNode_NewOperationNode
(
    AL_EXP_OP op    // Operation to perform.
);

// Create a new AlgebraicExpression operand node.
AlgebraicExpressionNode *AlgebraicExpressionNode_NewOperandNode
(
    GrB_Matrix mat,     // Matrix.
    bool free,          // Should operand be free when we're done.
    bool diagonal,      // Is operand a diagonal matrix?
    const char *src,    // Operand row domain (src node).
    const char *dest,   // Operand column domain (destination node).
    const char *edge    // Operand alias (edge).
);

//------------------------------------------------------------------------------
// AlgebraicExpression attributes.
//------------------------------------------------------------------------------

// Returns the source entity represented by the left-most operand row domain.
const char *AlgebraicExpressionNode_Source
(
    AlgebraicExpressionNode *root   // Root of expression.
);

// Returns the destination entity represented by the right-most operand column domain.
const char *AlgebraicExpressionNode_Destination
(
    AlgebraicExpressionNode *root   // Root of expression.
);

//------------------------------------------------------------------------------
// AlgebraicExpression modification functions.
//------------------------------------------------------------------------------

// Adds child node to root children list.
void AlgebraicExpressionNode_AddChild
(
    AlgebraicExpressionNode *root,  // Root to attach child to.
    AlgebraicExpressionNode *child  // Child node to attach.
);

// Evaluate expression tree.
void AlgebraicExpression_Eval
(
    const AlgebraicExpressionNode *exp, // Root node.
    GrB_Matrix res                      // Result output.
);

//------------------------------------------------------------------------------
// AlgebraicExpression debugging utilities.
//------------------------------------------------------------------------------

// Print algebraic expression to stdout.
void AlgebraicExpression_Print
(
    const AlgebraicExpressionNode *exp  // Root node.
);

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
	const char *src;                        // Alias of source node.
	const char *dest;                       // Alias of destination node.
	const char *edge;                       // Alias of sole edge operand, if one is present.
} AlgebraicExpression;

/* Constructs an empty expression. */
AlgebraicExpression *AlgebraicExpression_Empty(void);

/* Construct algebraic expression(s) from query graph. */
AlgebraicExpressionNode **AlgebraicExpression_FromQueryGraph(
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

void AlgebraicExpression_Free(AlgebraicExpression *ae);

#endif

