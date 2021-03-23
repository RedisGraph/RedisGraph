/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../path_patterns/path_pattern_ctx.h"
#include "../graph/query_graph.h"
#include "../graph/graph.h"
#include "algebraic_expression_structs.h"

//------------------------------------------------------------------------------
// AlgebraicExpression construction.
//------------------------------------------------------------------------------

// Construct algebraic expression form query graph.
AlgebraicExpression **AlgebraicExpression_FromQueryGraph
(
	const QueryGraph *qg    // Query-graph to process
);

// Construct algebraic expression form ebnf expression.
AlgebraicExpression *AlgebraicExpression_FromEbnf(
        const EBNFBase *ebnf
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
	const char *label,   // Label attached to matrix.
	AlgExpReference ref
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
// Algebraic expression reference creation functions.
//------------------------------------------------------------------------------
/* Creates reference that doesn't point to anything */
AlgExpReference AlgExpReference_NewEmpty();

/* Creates references to specified named path pattern */
AlgExpReference AlgExpReference_New(
	const char *name,
	bool transposed
);

/* Frees reference (but not referred named path pattern) */
void AlgExpReference_Free(AlgExpReference ref);

/* Clones reference (but not referred named path pattern)*/
AlgExpReference AlgExpReference_Clone(const AlgExpReference *ref);

/* Checks whether given operand is reference */
bool AlgebraicExpression_OperandIsReference(
		const AlgebraicExpression *root
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

// Remove source of algebraic expression from root.
AlgebraicExpression *AlgebraicExpression_RemoveSource
(
	AlgebraicExpression **root   // Root from which to remove a child.
);

// Remove destination of algebraic expression from root.
AlgebraicExpression *AlgebraicExpression_RemoveDest
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

// Evaluate expression tree. Because of algorithm
// that solves problem given by named path patterns,
// we need to update some matrices corresponds to them
// during expression evaluation. So here we use additional
// parameter PathPatternCtx.
void AlgebraicExpression_Eval
(
	const AlgebraicExpression *exp,
	GrB_Matrix res,
	PathPatternCtx *pathCtx
);

// Locates operand based on row,column domain and edge
// sets 'operand' to if found otherwise set it to NULL
// sets 'parent' if requested, parent can still be set to NULL
// if 'root' is the seeked operand
bool AlgebraicExpression_LocateOperand
(
	AlgebraicExpression *root,       // Root to search
	AlgebraicExpression **operand,   // [output] set to operand, NULL if missing
	AlgebraicExpression **parent,    // [output] set to operand parent
	const char *row_domain,          // operand row domain
	const char *column_domain,       // operand column domain
	const char *edge                 // operand edge name
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

char *AlgebraicExpression_ToStringDebug
(
	const AlgebraicExpression *exp  // Root node.
);

void _AlgebraicExpression_TotalShow(
		const AlgebraicExpression *exp  // Root node.
);

//------------------------------------------------------------------------------
// AlgebraicExpression optimizations
//------------------------------------------------------------------------------
void AlgebraicExpression_Optimize
(
	AlgebraicExpression **exp   // Expression to optimize.
);

// Push down transpose operations to individual operands.
void AlgebraicExpression_PushDownTranspose
(
	AlgebraicExpression *root   // Expression to modify
);

//------------------------------------------------------------------------------
// AlgebraicExpression free
//------------------------------------------------------------------------------

// Free algebraic expression.
void AlgebraicExpression_Free
(
	AlgebraicExpression *root  // Root node.
);

