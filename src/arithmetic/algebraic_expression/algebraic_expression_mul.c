/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "utils.h"
#include "../../query_ctx.h"
#include "../algebraic_expression.h"

GrB_Matrix _Eval_Mul
(
	const AlgebraicExpression *exp,
	GrB_Matrix res
) {
	//--------------------------------------------------------------------------
	// validate expression
	//--------------------------------------------------------------------------

	ASSERT(exp != NULL) ;
	ASSERT(AlgebraicExpression_ChildCount(exp) > 1) ;
	ASSERT(AlgebraicExpression_OperationCount(exp, AL_EXP_MUL) == 1) ;

	GrB_Info             info    ;
	GrB_Matrix           GrB_A   ;  // left operand
	GrB_Matrix           GrB_B   ;  // right operand
	RG_Matrix            RG_B    ;  // right operand
	GrB_Index            nvals   ;  // NNZ in res
	AlgebraicExpression  *left   ;  // left child
	AlgebraicExpression  *right  ;  // right child

	UNUSED(info) ;

	left = CHILD_AT(exp, 0) ;
	ASSERT(left->type == AL_OPERAND) ;

	// leftmost operand is expected to be a GrB_Matrix
	AlgebraicExpressionMatrixType A_MatType = left->operand.type ;
	ASSERT(A_MatType == AL_GrB_MAT) ;

	GrB_A = left->operand.grb_matrix ;

	GrB_Semiring semiring = GxB_ANY_PAIR_BOOL ;
	uint child_count = AlgebraicExpression_ChildCount(exp) ;

	// scan through children 1..n
	// perform GrB_mxm or RG_mxm depending on the type of operand i
	for(uint i = 1; i < child_count; i++) {
		right = CHILD_AT(exp, i) ;

		switch(right->operand.type) {
			case AL_RG_MAT:
				RG_B = right->operand.rg_matrix ;
				info = RG_mxm(res, semiring, GrB_A, RG_B) ;
				ASSERT(info == GrB_SUCCESS) ;
				break ;
			case AL_GrB_MAT:
				GrB_B = right->operand.grb_matrix ;
				info = GrB_mxm(res, NULL, NULL, semiring, GrB_A, GrB_B, NULL) ;
				ASSERT(info == GrB_SUCCESS) ;
				break ;
			default:
				// A is a RG_Matrix, shouldn't happen!
				ASSERT(false) ;
				break ;
		}

		GrB_Matrix_nvals(&nvals, res) ;
		if(nvals == 0) break ;

		// set A to res, preparation for next iteration
		GrB_A = res ;
	}

	return res ;
}

