/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../graph/query_graph.h"
#include "../graph/graph.h"

// Matrix, vector operations.
typedef enum {
	AL_EXP_ADD = 1,                 // Matrix addition.
	AL_EXP_MUL = (1 << 1),          // Matrix multiplication.
	AL_EXP_POW = (1 << 2),          // Matrix raised to a power.
	AL_EXP_TRANSPOSE = (1 << 3),    // Matrix transpose.
} AL_EXP_OP;

#define AL_EXP_ALL (AL_EXP_ADD | AL_EXP_MUL | AL_EXP_POW | AL_EXP_TRANSPOSE)

/* Algebraic operation, describes one of the following operations:
 * Matrix matrix addition
 * Matrix matrix multiplication
 * Matrix masking
 * Matrix transpose */
struct AlgebraicOperation {
    union{
        struct {
            GrB_Matrix Mask;
            GrB_Semiring semiring;
        } add;
        struct {
            GrB_Matrix Mask;
            GrB_Semiring semiring;
        } mul;
        struct {
            GrB_Matrix Mask;
            GrB_Semiring semiring;
        } pow;
        struct {
            GrB_Matrix Mask;
            GrB_Semiring semiring;
        } transpose;
    };
    AL_EXP_OP type;
};

// Type of node within an algebraic expression
typedef enum {
	AL_OPERAND,
	AL_OPERATION,
} AlgebraicExpressionType;

/* Forward declarations. */
typedef struct AlgebraicExpression AlgebraicExpression;

struct AlgebraicExpression {
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
	const QueryGraph *qg,   // Query-graph to process
	uint *exp_count         // Number of algebraic expressions generated.
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
    bool free,          // Should operand be free when we're done.
    bool diagonal,      // Is operand a diagonal matrix?
    const char *src,    // Operand row domain (src node).
    const char *dest,   // Operand column domain (destination node).
    const char *edge    // Operand alias (edge).
);

// Clone algebraic expression node.
AlgebraicExpression *AlgebraicExpression_Clone
(
    const AlgebraicExpression *exp  // Expression to clone.
);

//------------------------------------------------------------------------------
// AlgebraicExpression attributes.
//------------------------------------------------------------------------------

// Returns the source entity represented by the left-most operand row domain.
const char *AlgebraicExpression_Source
(
    AlgebraicExpression *root   // Root of expression.
);

// Returns the destination entity represented by the right-most operand column domain.
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

// Returns the number of operations in expression.
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

// Returns true if expression contains operation.
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
    AlgebraicExpression *root   // Root from which to remove a child.
);

// Remove rightmost child node from root.
AlgebraicExpression *AlgebraicExpression_RemoveRightmostNode
(
    AlgebraicExpression *root   // Root from which to remove a child.
);

// Multiply expression to the left by operand
// A * (exp)
void AlgebraicExpression_MultiplyToTheLeft
(
    AlgebraicExpression **root,
    GrB_Matrix m
);

// Multiply expression to the right by operand
// (exp) * A
void AlgebraicExpression_MultiplyToTheRight
(
    AlgebraicExpression **root,
	GrB_Matrix m
);

// Add expression to the left by operand
// A + (exp)
void AlgebraicExpression_AddToTheLeft
(
    AlgebraicExpression **root,
    GrB_Matrix m
);

// Add expression to the right by operand
// (exp) + A
void AlgebraicExpression_AddToTheRight
(
    AlgebraicExpression **root,
	GrB_Matrix m
);

// Transpose expression
// T(T(A)) = A
// T(A + B) = T(A) + T(B)
// T(A * B) = T(B) * T(B)
void AlgebraicExpression_Transpose
(
    AlgebraicExpression *exp    // Expression to transpose.
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
