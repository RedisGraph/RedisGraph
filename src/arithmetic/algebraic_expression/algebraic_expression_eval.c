/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../algebraic_expression.h"

static GrB_Matrix _Eval_Transpose(const AlgebraicExpression *exp,
								  GrB_Matrix res) {
	assert(exp && AlgebraicExpression_ChildCount(exp) == 1);

	AlgebraicExpression *child = exp->operation.children[0];
	assert(child->type == AL_OPERAND);
	GrB_Info info = GrB_transpose(res, GrB_NULL, GrB_NULL, child->operand.matrix, GrB_NULL);
	assert(info == GrB_SUCCESS);
	return res;
}

static GrB_Matrix _Eval_Add(const AlgebraicExpression *exp, GrB_Matrix res) {
	assert(exp && AlgebraicExpression_ChildCount(exp) > 1);

	GrB_Index nrows;
	GrB_Index ncols;
	GrB_Matrix inter;
	GrB_Matrix_nrows(&nrows, res);
	GrB_Matrix_ncols(&ncols, res);
	GrB_Matrix_new(&inter, GrB_BOOL, nrows, ncols);

	AlgebraicExpression_Eval(exp->operation.children[0], res);

	uint child_count = AlgebraicExpression_ChildCount(exp);
	for(uint i = 1; i < child_count; i++) {
		AlgebraicExpression_Eval(exp->operation.children[i], inter);
		// Perform addition.
		assert(GrB_eWiseAdd_Matrix_Semiring(res, GrB_NULL, GrB_NULL, GxB_LAND_LOR_BOOL, res, inter,
											GrB_NULL) == GrB_SUCCESS);
	}

	GrB_Matrix_free(&inter);
	return res;
}

// static GrB_Matrix _Eval_MUL(AlgebraicExpression *exp, GrB_Matrix res) {
// 	// Expression already evaluated.
// 	if(exp->operation.v != NULL) return exp->operation.v;

// 	GrB_Descriptor desc = NULL; // Descriptor used for transposing.
// 	AlgebraicExpression *rightHand = exp->operation.r;
// 	AlgebraicExpression *leftHand = exp->operation.l;

// 	// Determine if left or right expression needs to be transposed.
// 	if(leftHand && leftHand->type == AL_OPERATION && leftHand->operation.op == AL_EXP_TRANSPOSE) {
// 		if(!desc) GrB_Descriptor_new(&desc);
// 		GrB_Descriptor_set(desc, GrB_INP0, GrB_TRAN);
// 	}

// 	if(rightHand && rightHand->type == AL_OPERATION && rightHand->operation.op == AL_EXP_TRANSPOSE) {
// 		if(!desc) GrB_Descriptor_new(&desc);
// 		GrB_Descriptor_set(desc, GrB_INP1, GrB_TRAN);
// 	}

// 	// Evaluate right left expressions.
// 	GrB_Matrix r = AlgebraicExpression_Eval(exp->operation.r, res);
// 	GrB_Matrix l = AlgebraicExpression_Eval(exp->operation.l, res);

// 	// Perform multiplication.
// 	assert(GrB_mxm(res, NULL, NULL, GxB_LAND_LOR_BOOL, l, r, desc) == GrB_SUCCESS);

// 	// Store intermidate if expression is marked for reuse.
// 	if(exp->operation.reusable) {
// 		assert(exp->operation.v == NULL);
// 		GrB_Matrix_dup(&exp->operation.v, res);
// 	}

// 	if(desc) GrB_Descriptor_free(&desc);
// 	return res;
// }

void AlgebraicExpression_Eval(const AlgebraicExpression *exp, GrB_Matrix res) {
	assert(exp);

	// Perform operation.
	switch(exp->type) {
	case AL_OPERATION:
		switch(exp->operation.op) {
		case AL_EXP_MUL:
			// _Eval_Mul(exp, res);
			break;

		case AL_EXP_ADD:
			_Eval_Add(exp, res);
			break;

		case AL_EXP_TRANSPOSE:
			_Eval_Transpose(exp, res);
			break;

		default:
			assert("Unknown algebraic expression operation" && false);
		}
	case AL_OPERAND:
		assert(false);
	}
}
