/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "utils.h"
#include "../../query_ctx.h"
#include "../algebraic_expression.h"

// forward declarations
GrB_Matrix _AlgebraicExpression_Eval
(
	const AlgebraicExpression *exp,
	GrB_Matrix res
);

static GrB_Matrix _Eval_Transpose
(
	const AlgebraicExpression *exp,
	GrB_Matrix res
) {
	// this function is currently unused
	ASSERT(exp && AlgebraicExpression_ChildCount(exp) == 1);

	AlgebraicExpression *child = FIRST_CHILD(exp);
	ASSERT(child->type == AL_OPERAND);
	ASSERT(child->operand.type == AL_GrB_MAT);
	GrB_Info info = GrB_transpose(res, NULL, NULL, child->operand.grb_matrix, NULL);
	ASSERT(info == GrB_SUCCESS);
	return res;
}

static GrB_Matrix _Eval_Add
(
	const AlgebraicExpression *exp,
	GrB_Matrix res
) {
	ASSERT(exp && AlgebraicExpression_ChildCount(exp) > 1);

	GrB_Info info;
	UNUSED(info);
	GrB_Index nrows;                // number of rows of operand
	GrB_Index ncols;                // number of columns of operand
	bool res_in_use = false;        // can we use `res` for intermediate evaluation
	GrB_Matrix a = NULL;            // left operand
	GrB_Matrix b = NULL;            // right operand
	GrB_Matrix inter = NULL;        // intermediate matrix
	GrB_Descriptor desc = NULL;     // descriptor used for transposing operands (currently unused)

	// get left and right operands
	AlgebraicExpression *left = CHILD_AT(exp, 0);
	AlgebraicExpression *right = CHILD_AT(exp, 1);

	// if left operand is a matrix, simply get it
	// otherwise evaluate left hand side using `res` to store LHS value
	if(left->type == AL_OPERAND) {
		ASSERT(left->operand.type == AL_GrB_MAT);
		a = left->operand.grb_matrix;
	} else {
		if(left->operation.op == AL_EXP_TRANSPOSE) {
			ASSERT(AlgebraicExpression_ChildCount(left) == 1);
			a = left->operation.children[0]->operand.grb_matrix;
			if(desc == NULL) GrB_Descriptor_new(&desc);
			GrB_Descriptor_set(desc, GrB_INP0, GrB_TRAN);
		} else {
			a = _AlgebraicExpression_Eval(left, res);
			res_in_use = true;
		}
	}

	// if right operand is a matrix, simply get it
	// otherwise evaluate right hand side using `res`
	// if free or create an additional matrix to store RHS value
	if(right->type == AL_OPERAND) {
		ASSERT(right->operand.type == AL_GrB_MAT);
		b = right->operand.grb_matrix;
	} else {
		if(right->operation.op == AL_EXP_TRANSPOSE) {
			ASSERT(AlgebraicExpression_ChildCount(right) == 1);
			b = right->operation.children[0]->operand.grb_matrix;
			if(desc == NULL) GrB_Descriptor_new(&desc);
			GrB_Descriptor_set(desc, GrB_INP1, GrB_TRAN);
		} else if(res_in_use) {
			// `res` is in use, create an additional matrix
			GrB_Matrix_nrows(&nrows, a);
			GrB_Matrix_ncols(&ncols, a);
			info = GrB_Matrix_new(&inter, GrB_BOOL, nrows, ncols);
			ASSERT(info == GrB_SUCCESS);
			b = _AlgebraicExpression_Eval(right, inter);
		} else {
			// `res` is not used just yet, use it for RHS evaluation
			b = _AlgebraicExpression_Eval(right, res);
		}
	}

	// perform addition
	info = GrB_eWiseAdd(res, NULL, NULL, GxB_ANY_PAIR_BOOL, a, b, desc);
	ASSERT(info == GrB_SUCCESS);

	// reset descriptor if non-null
	if(desc != NULL) GrB_Descriptor_set(desc, GrB_INP0, GxB_DEFAULT);

	uint child_count = AlgebraicExpression_ChildCount(exp);
	// expression has more than 2 operands, e.g. A+B+C...
	for(uint i = 2; i < child_count; i++) {
		// reset descriptor if non-null
		if(desc != NULL) GrB_Descriptor_set(desc, GrB_INP1, GxB_DEFAULT);
		right = CHILD_AT(exp, i);

		if(right->type == AL_OPERAND) {
			ASSERT(right->operand.type == AL_GrB_MAT);
			b = right->operand.grb_matrix;
		} else {
			if(right->operation.op == AL_EXP_TRANSPOSE) {
				ASSERT(AlgebraicExpression_ChildCount(right) == 1);
				b = right->operation.children[0]->operand.grb_matrix;
				if(desc == NULL) GrB_Descriptor_new(&desc);
				GrB_Descriptor_set(desc, GrB_INP1, GrB_TRAN);
			} else {
				// 'right' represents either + or * operation
				if(inter == NULL) {
					// can't use `res`, use an intermidate matrix
					GrB_Matrix_nrows(&nrows, res);
					GrB_Matrix_ncols(&ncols, res);
					info = GrB_Matrix_new(&inter, GrB_BOOL, nrows, ncols);
					ASSERT(info == GrB_SUCCESS);
				}
				b = _AlgebraicExpression_Eval(right, inter);
			}
		}

		// perform addition
		info = GrB_eWiseAdd(res, NULL, NULL, GxB_ANY_PAIR_BOOL, res, b, NULL);
		ASSERT(info == GrB_SUCCESS);
	}

	if(inter != NULL) GrB_Matrix_free(&inter);
	if(desc != NULL) GrB_free(&desc);
	return res;
}

static GrB_Matrix _Eval_Mul
(
	const AlgebraicExpression *exp,
	GrB_Matrix res
) {
	ASSERT(exp &&
		   AlgebraicExpression_ChildCount(exp) > 1 &&
		   AlgebraicExpression_OperationCount(exp, AL_EXP_MUL) == 1);

	GrB_Info info;
	UNUSED(info);
	GrB_Matrix A;
	RG_Matrix B;
	GrB_Index nvals;
	GrB_Descriptor desc = NULL;
	AlgebraicExpression *left = CHILD_AT(exp, 0);
	AlgebraicExpression *right = CHILD_AT(exp, 1);

	if(left->type == AL_OPERATION) {
		ASSERT(left->operation.op == AL_EXP_TRANSPOSE);
		ASSERT(AlgebraicExpression_ChildCount(left) == 1);
		if(desc == NULL) GrB_Descriptor_new(&desc);
		GrB_Descriptor_set(desc, GrB_INP0, GrB_TRAN);
		left = CHILD_AT(left, 0);
	}
	ASSERT(left->operand.type == AL_GrB_MAT);
	A = left->operand.grb_matrix;

	if(right->type == AL_OPERATION) {
		ASSERT(right->operation.op == AL_EXP_TRANSPOSE);
		ASSERT(AlgebraicExpression_ChildCount(right) == 1);
		if(desc == NULL) GrB_Descriptor_new(&desc);
		GrB_Descriptor_set(desc, GrB_INP1, GrB_TRAN);
		right = CHILD_AT(right, 0);
	}

	if(right->operand.type == AL_GrB_MAT) {
		ASSERT(right->operand.grb_matrix == IDENTITY_MATRIX);
		// reset descriptor
		// as the identity matrix does not need to be transposed
		if(desc != NULL) GrB_Descriptor_set(desc, GrB_INP1, GxB_DEFAULT);
		// B is the identity matrix, Perform A * I
		info = GrB_Matrix_apply(res, NULL, NULL, GrB_IDENTITY_BOOL, A, desc);
		ASSERT(info == GrB_SUCCESS);
	} else {
		B = right->operand.rg_matrix;
		// perform multiplication
		info = RG_mxm(res, GxB_ANY_PAIR_BOOL, A, B);
		ASSERT(info == GrB_SUCCESS);
	}

	GrB_wait(&res);

	// reset descriptor if non-null
	if(desc != NULL) GrB_Descriptor_set(desc, GrB_INP0, GxB_DEFAULT);

	uint child_count = AlgebraicExpression_ChildCount(exp);
	for(uint i = 2; i < child_count; i++) {
		// reset descriptor if non-null
		if(desc != NULL) GrB_Descriptor_set(desc, GrB_INP1, GxB_DEFAULT);

		right = CHILD_AT(exp, i);
		if(right->type == AL_OPERATION) {
			ASSERT(right->operation.op == AL_EXP_TRANSPOSE);
			ASSERT(AlgebraicExpression_ChildCount(right) == 1);
			if(desc == NULL) GrB_Descriptor_new(&desc);
			GrB_Descriptor_set(desc, GrB_INP1, GrB_TRAN);
			right = CHILD_AT(right, 0);
		}
		ASSERT(right->operand.type == AL_RG_MAT);
		B = right->operand.rg_matrix;

		// perform multiplication
		info = RG_mxm(res, GxB_ANY_PAIR_BOOL, res, B);
		ASSERT(info == GrB_SUCCESS);

		GrB_Matrix_nvals(&nvals, res);
		if(nvals == 0) break;
	}

	if(desc != NULL) GrB_free(&desc);

	return res;
}

GrB_Matrix _AlgebraicExpression_Eval
(
	const AlgebraicExpression *exp,
	GrB_Matrix res
) {
	ASSERT(exp);

	// perform operation
	switch(exp->type) {
	case AL_OPERATION:
		switch(exp->operation.op) {
		case AL_EXP_MUL:
			res = _Eval_Mul(exp, res);
			break;

		case AL_EXP_ADD:
			res = _Eval_Add(exp, res);
			break;

		case AL_EXP_TRANSPOSE:
			res = _Eval_Transpose(exp, res);
			break;

		default:
			ASSERT("Unknown algebraic expression operation" && false);
		}
		break;
	case AL_OPERAND:
		ASSERT(exp->operand.type == AL_GrB_MAT);
		res = exp->operand.grb_matrix;
		break;
	default:
		ASSERT("Unknown algebraic expression node type" && false);
	}

	return res;
}

void AlgebraicExpression_Eval
(
	const AlgebraicExpression *exp,
	GrB_Matrix res
) {
	ASSERT(exp && exp->type == AL_OPERATION);
	_AlgebraicExpression_Eval(exp, res);
}

