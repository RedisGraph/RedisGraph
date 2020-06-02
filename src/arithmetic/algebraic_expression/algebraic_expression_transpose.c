/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../algebraic_expression.h"
#include "utils.h"
#include "../../util/arr.h"

/* Transpose expression
 * Wraps expression with a transpose operation
 * Transpose(exp) */
void AlgebraicExpression_Transpose
(
	AlgebraicExpression **exp    // Expression to transpose.
) {
	assert(exp);
	AlgebraicExpression *root = AlgebraicExpression_NewOperation(AL_EXP_TRANSPOSE);
	AlgebraicExpression_AddChild(root, *exp);
	*exp = root;
}
