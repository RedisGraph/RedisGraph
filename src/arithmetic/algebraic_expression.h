/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../graph/query_graph.h"
#include "../graph/graph.h"

static GrB_Matrix IDENTITY_MATRIX = (GrB_Matrix)0x31032017;  // Identity matrix.

// Matrix, vector operations.
typedef enum {
	AL_EXP_ADD = 1,                 // Matrix addition.
	AL_EXP_MUL = (1 << 1),          // Matrix multiplication.
	AL_EXP_POW = (1 << 2),          // Matrix raised to a power.
	AL_EXP_TRANSPOSE = (1 << 3),    // Matrix transpose.
} AL_EXP_OP;

#define AL_EXP_ALL (AL_EXP_ADD | AL_EXP_MUL | AL_EXP_POW | AL_EXP_TRANSPOSE)

// Type of node within an algebraic expression
typedef enum {
	AL_OPERAND = 1,
	AL_OPERATION  = (1 << 1),
} AlgebraicExpressionType;

/* Forward declarations. */
typedef struct AlgebraicExpression AlgebraicExpression;

struct AlgebraicExpression {
	union {
		struct {
			bool diagonal;          // Diagonal matrix.
			bool bfree;             // If the matrix is scoped to this expression, it should be freed with it.
			GrB_Matrix matrix;      // Matrix operand.
			const char *src;        // Alias given to operand's rows (src node).
			const char *dest;       // Alias given to operand's columns (destination node).
			const char *edge;       // Alias given to operand (edge).
			const char *label;      // Label attached to matrix.
		} operand;
		struct {
			AL_EXP_OP op;                   // Operation: `*`,`+`,`transpose`
			AlgebraicExpression **children; // Child nodes.
		} operation;
	};
	AlgebraicExpressionType type;   // Type of node, either an operation or an operand.
};

//------------------------------------------------------------------------------
// AlgebraicExpression construction.
//------------------------------------------------------------------------------

// Construct algebraic expression form query graph.
AlgebraicExpression **AlgebraicExpression_FromQueryGraph
(
	const QueryGraph *qg    // Query-graph to process
);

//------------------------------------------------------------------------------
// AlgebraicExpression Node creation functions.
//------------------------------------------------------------------------------

// Create a new AlgebraicExpression operation node.
AlgebraicExpression *AlgebraicExpression_NewOperation
(
	AL_EXP_OP op    // Operation to perform.
);

// Create a new AlgebraicExpression operand node.
AlgebraicExpression *AlgebraicExpression_NewOperand
(
	GrB_Matrix mat,     // Matrix.
	bool diagonal,      // Is operand a diagonal matrix?
	const char *src,    // Operand row domain (src node).
	const char *dest,   // Operand column domain (destination node).
	const char *edge,   // Operand alias (edge).
	const char *label   // Label attached to matrix.
);

// Clone algebraic expression node.
AlgebraicExpression *AlgebraicExpression_Clone
(
	const AlgebraicExpression *exp  // Expression to clone.
);

//------------------------------------------------------------------------------
// AlgebraicExpression attributes.
//------------------------------------------------------------------------------

// Returns the source entity alias represented by the left-most operand (row domain).
const char *AlgebraicExpression_Source
(
	AlgebraicExpression *root   // Root of expression.
);

// Returns the destination entity alias represented by the right-most operand (column domain).
const char *AlgebraicExpression_Destination
(
	AlgebraicExpression *root   // Root of expression.
);

/* Returns the first edge alias encountered.
 * if no edge alias is found NULL is returned. */
const char *AlgebraicExpression_Edge
(
	const AlgebraicExpression *root   // Root of expression.
);

// Returns the number of child nodes directly under root.
uint AlgebraicExpression_ChildCount
(
	const AlgebraicExpression *root   // Root of expression.
);

// Returns the number of operands in expression.
uint AlgebraicExpression_OperandCount
(
	const AlgebraicExpression *root   // Root of expression.
);

// Returns the number of operations of given type in expression.
uint AlgebraicExpression_OperationCount
(
	const AlgebraicExpression *root,    // Root of expression.
	AL_EXP_OP op_type                   // Type of operation.
);

// Returns true if entire expression is transposed.
bool AlgebraicExpression_Transposed
(
	const AlgebraicExpression *root   // Root of expression.
);

// Returns true if expression contains an operation of type `op`.
bool AlgebraicExpression_ContainsOp
(
	const AlgebraicExpression *root,    // Root of expression.
	AL_EXP_OP op                        // Operation to look for.
);

// Checks to see if operand at position `operand_idx` is a diagonal matrix.
bool AlgebraicExpression_DiagonalOperand
(
	const AlgebraicExpression *root,    // Root of expression.
	uint operand_idx                    // Operand position (LTR, zero based).
);

//------------------------------------------------------------------------------
// AlgebraicExpression modification functions.
//------------------------------------------------------------------------------

// Adds child node to root children list.
void AlgebraicExpression_AddChild
(
	AlgebraicExpression *root,  // Root to attach child to.
	AlgebraicExpression *child  // Child node to attach.
);

// Remove leftmost child node from root.
AlgebraicExpression *AlgebraicExpression_RemoveLeftmostNode
(
	AlgebraicExpression **root   // Root from which to remove a child.
);

// Remove rightmost child node from root.
AlgebraicExpression *AlgebraicExpression_RemoveRightmostNode
(
	AlgebraicExpression **root   // Root from which to remove a child.
);

// Multiply expression to the left by operand
// m * (exp)
void AlgebraicExpression_MultiplyToTheLeft
(
	AlgebraicExpression **root,
	GrB_Matrix m
);

// Multiply expression to the right by operand
// (exp) * m
void AlgebraicExpression_MultiplyToTheRight
(
	AlgebraicExpression **root,
	GrB_Matrix m
);

// Add expression to the left by operand
// m + (exp)
void AlgebraicExpression_AddToTheLeft
(
	AlgebraicExpression **root,
	GrB_Matrix m
);

// Add expression to the right by operand
// (exp) + m
void AlgebraicExpression_AddToTheRight
(
	AlgebraicExpression **root,
	GrB_Matrix m
);

// Transpose expression
// By wrapping exp in a transpose root node.
void AlgebraicExpression_Transpose
(
	AlgebraicExpression **exp    // Expression to transpose.
);

// Evaluate expression tree.
void AlgebraicExpression_Eval
(
	const AlgebraicExpression *exp, // Root node.
	GrB_Matrix res                  // Result output.
);

//------------------------------------------------------------------------------
// AlgebraicExpression debugging utilities.
//------------------------------------------------------------------------------

// Create an algebraic expression from string
// e.g. B*T(B+A)
AlgebraicExpression *AlgebraicExpression_FromString
(
	const char *exp,    // String representation of expression.
	rax *matrices       // Map of matrices referred to in expression.
);

// Print a tree structure of algebraic expression to stdout.
void AlgebraicExpression_PrintTree
(
	const AlgebraicExpression *exp  // Root node.
);

// Print algebraic expression to stdout.
void AlgebraicExpression_Print
(
	const AlgebraicExpression *exp  // Root node.
);

// Return a string representation of expression.
char *AlgebraicExpression_ToString
(
	const AlgebraicExpression *exp  // Root node.
);

//------------------------------------------------------------------------------
// AlgebraicExpression optimizations
//------------------------------------------------------------------------------
void AlgebraicExpression_Optimize
(
	AlgebraicExpression **exp   // Expression to optimize.
);

//------------------------------------------------------------------------------
// AlgebraicExpression free
//------------------------------------------------------------------------------

// Free algebraic expression.
void AlgebraicExpression_Free
(
	AlgebraicExpression *root  // Root node.
);

