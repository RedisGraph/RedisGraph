/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "utils.h"
#include "../../query_ctx.h"
#include "../algebraic_expression.h"

GrB_Matrix _Eval_Add
(
	const AlgebraicExpression *exp,
	GrB_Matrix res
) {
	ASSERT(exp && AlgebraicExpression_ChildCount(exp) > 1);

	GrB_Info info;
	UNUSED(info);
	GrB_Index nrows;                   // number of rows of operand
	GrB_Index ncols;                   // number of columns of operand

	bool        res_in_use  =  false;  //  can we use `res` for intermediate evaluation
	GrB_Matrix  a           =  NULL;   //  left operand
	GrB_Matrix  b           =  NULL;   //  right operand
	GrB_Matrix  inter       =  NULL;   //  intermediate matrix

	// get left and right operands
	AlgebraicExpression *left = CHILD_AT(exp, 0);
	AlgebraicExpression *right = CHILD_AT(exp, 1);

	// if left operand is a matrix, simply get it
	// otherwise evaluate left hand side using `res` to store LHS value
	if(left->type == AL_OPERAND) {
		ASSERT(left->operand.type == AL_GrB_MAT);
		a = left->operand.grb_matrix;
	} else {
		AlgebraicExpression_Eval(left, res);
		a = res;
		res_in_use = true;
	}

	// if right operand is a matrix, simply get it
	// otherwise evaluate right hand side using `res`
	// if free or create an additional matrix to store RHS value
	if(right->type == AL_OPERAND) {
		ASSERT(right->operand.type == AL_GrB_MAT);
		b = right->operand.grb_matrix;
	} else {
		if(res_in_use) {
			// `res` is in use, create an additional matrix
			GrB_Matrix_nrows(&nrows, a);
			GrB_Matrix_ncols(&ncols, a);
			info = GrB_Matrix_new(&inter, GrB_BOOL, nrows, ncols);
			ASSERT(info == GrB_SUCCESS);
			AlgebraicExpression_Eval(right, inter);
			b = inter;
		} else {
			// `res` is not used just yet, use it for RHS evaluation
			AlgebraicExpression_Eval(right, res);
			b = res;
		}
	}

	// perform addition
	info = GrB_eWiseAdd(res, NULL, NULL, GxB_ANY_PAIR_BOOL, a, b, NULL);
	ASSERT(info == GrB_SUCCESS);

	uint child_count = AlgebraicExpression_ChildCount(exp);
	// expression has more than 2 operands, e.g. A+B+C...
	for(uint i = 2; i < child_count; i++) {
		right = CHILD_AT(exp, i);

		if(right->type == AL_OPERAND) {
			ASSERT(right->operand.type == AL_GrB_MAT);
			b = right->operand.grb_matrix;
		} else {
			// 'right' represents either + or * operation
			if(inter == NULL) {
				// can't use `res`, use an intermidate matrix
				GrB_Matrix_nrows(&nrows, res);
				GrB_Matrix_ncols(&ncols, res);
				info = GrB_Matrix_new(&inter, GrB_BOOL, nrows, ncols);
				ASSERT(info == GrB_SUCCESS);
			}
			AlgebraicExpression_Eval(right, inter);
			b = inter;
		}

		// perform addition
		info = GrB_eWiseAdd(res, NULL, NULL, GxB_ANY_PAIR_BOOL, res, b, NULL);
		ASSERT(info == GrB_SUCCESS);
	}

	if(inter != NULL) GrB_Matrix_free(&inter);
	return res;
}

