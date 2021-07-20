/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "utils.h"
#include "../../query_ctx.h"
#include "../algebraic_expression.h"

RG_Matrix _Eval_Mul
(
	const AlgebraicExpression *exp,
	RG_Matrix res
) {
	//--------------------------------------------------------------------------
	// validate expression
	//--------------------------------------------------------------------------

	ASSERT(exp != NULL) ;
	ASSERT(AlgebraicExpression_ChildCount(exp) > 1) ;
	ASSERT(AlgebraicExpression_OperationCount(exp, AL_EXP_MUL) == 1) ;

	GrB_Info             info    ;
	RG_Matrix            A       ;  // left operand
	RG_Matrix            B       ;  // right operand
	GrB_Index            nvals   ;  // NNZ in res
	AlgebraicExpression  *left   ;  // left child
	AlgebraicExpression  *right  ;  // right child

	UNUSED(info) ;

	left = CHILD_AT(exp, 0) ;
	ASSERT(left->type == AL_OPERAND) ;

	A = left->operand.matrix ;

	GrB_Semiring semiring = GxB_ANY_PAIR_BOOL ;
	uint child_count = AlgebraicExpression_ChildCount(exp) ;

	// scan through children 1..n
	// perform RG_mxm
	for(uint i = 1; i < child_count; i++) {
		right = CHILD_AT(exp, i) ;
		B = right->operand.matrix ;
		info = RG_mxm(res, semiring, A, B) ;
		ASSERT(info == GrB_SUCCESS) ;

		RG_Matrix_nvals(&nvals, res) ;
		if(nvals == 0) break ;

		// set A to res, preparation for next iteration
		A = res ;
	}

	return res ;
}

