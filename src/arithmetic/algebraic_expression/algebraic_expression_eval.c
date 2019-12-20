/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../algebraic_expression.h"

// Forward declarations
GrB_Matrix _AlgebraicExpression_Eval(const AlgebraicExpression *exp, GrB_Matrix res);

static GrB_Matrix _Eval_Transpose
(
	const AlgebraicExpression *exp,
	GrB_Matrix res
) {
	assert(exp && AlgebraicExpression_ChildCount(exp) == 1);

	AlgebraicExpression *child = exp->operation.children[0];
	assert(child->type == AL_OPERAND);
	GrB_Info info = GrB_transpose(res, GrB_NULL, GrB_NULL, child->operand.matrix, GrB_NULL);
	assert(info == GrB_SUCCESS);
	return res;
}

static GrB_Matrix _Eval_Add(const AlgebraicExpression *exp, GrB_Matrix res) {
	assert(exp && AlgebraicExpression_ChildCount(exp) > 1);

	GrB_Index nrows;                // Number of rows of operand.
	GrB_Index ncols;                // Number of columns of operand.
	GrB_Matrix a = GrB_NULL;        // Left operand.
	GrB_Matrix b = GrB_NULL;        // Right operand.
	GrB_Matrix inter = GrB_NULL;    // Intermidate matrix.
	GrB_Descriptor desc = GrB_NULL; // Descriptor used for transposing operands.
	bool res_in_use = false;        // Can we use `res` for intermidate evaluation.

	GrB_Descriptor_new(&desc);

	// Get left and right operands.
	AlgebraicExpression *left = exp->operation.children[0];
	AlgebraicExpression *right = exp->operation.children[1];

	/* If left operand is a matrix, simply get it.
	 * Otherwise evaluate left hand side using `res` to store LHS value. */
	if(left->type == AL_OPERAND) {
		a = left->operand.matrix;
	} else {
		if(left->operation.op == AL_EXP_TRANSPOSE) {
			a = left->operation.children[0]->operand.matrix;
			GrB_Descriptor_set(desc, GrB_INP0, GrB_TRAN);
		} else {
			a = _AlgebraicExpression_Eval(exp->operation.children[0], res);
			res_in_use = true;
		}
	}

	/* If right operand is a matrix, simply get it.
	 * Otherwise evaluate right hand side using `res` if free or create an additional matrix to store RHS value. */
	if(right->type == AL_OPERAND) {
		b = right->operand.matrix;
	} else {
		if(right->operation.op == AL_EXP_TRANSPOSE) {
			b = right->operation.children[0]->operand.matrix;
			GrB_Descriptor_set(desc, GrB_INP1, GrB_TRAN);
		} else if(res_in_use) {
			// `res` is in use, create an additional matrix.
			GrB_Matrix_nrows(&nrows, a);
			GrB_Matrix_ncols(&ncols, a);
			GrB_Matrix_new(&inter, GrB_BOOL, nrows, ncols);
			b = _AlgebraicExpression_Eval(exp->operation.children[1], inter);
		} else {
			// `res` is not used just yet, use it for RHS evaluation.
			b = _AlgebraicExpression_Eval(exp->operation.children[1], res);
		}
	}

	// Perform addition.
	if(GrB_eWiseAdd_Matrix_Semiring(res, GrB_NULL, GrB_NULL, GxB_LAND_LOR_BOOL, a, b,
									desc) != GrB_SUCCESS) {
		printf("Failed adding operands, error:%s\n", GrB_error());
		assert(false);
	}

	// Reset descriptor.
	GrB_Descriptor_set(desc, GrB_INP0, GxB_DEFAULT);
	GrB_Descriptor_set(desc, GrB_INP1, GxB_DEFAULT);

	uint child_count = AlgebraicExpression_ChildCount(exp);
	// Expression has more than 2 operands, e.g. A+B+C...
	for(uint i = 2; i < child_count; i++) {
		right = exp->operation.children[i];

		if(right->type == AL_OPERAND) {
			b = right->operand.matrix;
		} else {
			if(right->operation.op == AL_EXP_TRANSPOSE) {
				b = right->operation.children[0]->operand.matrix;
				GrB_Descriptor_set(desc, GrB_INP1, GrB_TRAN);
			} else if(inter == GrB_NULL) {
				// Can't use `res`, use an intermidate matrix.
				GrB_Matrix_nrows(&nrows, res);
				GrB_Matrix_ncols(&ncols, res);
				GrB_Matrix_new(&inter, GrB_BOOL, nrows, ncols);
			}
			b = _AlgebraicExpression_Eval(right, inter);
		}

		// Perform addition.
		if(GrB_eWiseAdd_Matrix_Semiring(res, GrB_NULL, GrB_NULL, GxB_LAND_LOR_BOOL, res, b,
										GrB_NULL) != GrB_SUCCESS) {
			printf("Failed adding operands, error:%s\n", GrB_error());
			assert(false);
		}

        // Reset descriptor.
        GrB_Descriptor_set(desc, GrB_INP1, GxB_DEFAULT);
	}

	if(inter != GrB_NULL) GrB_Matrix_free(&inter);
	GrB_free(&desc);
	return res;
}

static GrB_Matrix _Eval_Mul(const AlgebraicExpression *exp, GrB_Matrix res) {
	assert(exp && AlgebraicExpression_ChildCount(exp) > 1);

	GrB_Index nrows;                // Number of rows of operand.
	GrB_Index ncols;                // Number of columns of operand.
	GrB_Matrix a = GrB_NULL;        // Left operand.
	GrB_Matrix b = GrB_NULL;        // Right operand.
	GrB_Matrix inter = GrB_NULL;    // Intermidate matrix.
	GrB_Descriptor desc = GrB_NULL; // Descriptor used for transposing operands.
	bool res_in_use = false;        // Can we use `res` for intermidate evaluation.

	// Get left and right operands.
	AlgebraicExpression *left = exp->operation.children[0];
	AlgebraicExpression *right = exp->operation.children[1];

	/* If left operand is a matrix, simply get it.
	 * Otherwise evaluate left hand side using `res` to store LHS value. */
	if(left->type == AL_OPERAND) {
		a = left->operand.matrix;
	} else {
		a = _AlgebraicExpression_Eval(exp->operation.children[0], res);
		res_in_use = true;
	}

	/* If right operand is a matrix, simply get it.
	 * Otherwise evaluate right hand side using `res` if free or create an additional matrix to store RHS value. */
	if(right->type == AL_OPERAND) {
		b = right->operand.matrix;
	} else {
		if(res_in_use) {
			// `res` is in use, create an additional matrix.
			GrB_Matrix_nrows(&nrows, a);
			GrB_Matrix_ncols(&ncols, a);
			GrB_Matrix_new(&inter, GrB_BOOL, nrows, ncols);
			b = _AlgebraicExpression_Eval(exp->operation.children[1], inter);
		} else {
			// `res` is not used just yet, use it for RHS evaluation.
			b = _AlgebraicExpression_Eval(exp->operation.children[1], res);
		}
	}

	// Perform addition.
	if(GrB_eWiseAdd_Matrix_Semiring(res, GrB_NULL, GrB_NULL, GxB_LAND_LOR_BOOL, a, b,
									GrB_NULL) != GrB_SUCCESS) {
		printf("Failed adding operands, error:%s\n", GrB_error());
		assert(false);
	}

	uint child_count = AlgebraicExpression_ChildCount(exp);
	// Expression has more than 2 operands, e.g. A+B+C...
	for(uint i = 2; i < child_count; i++) {
		right = exp->operation.children[i];

		if(right->type == AL_OPERAND) {
			b = right->operand.matrix;
		} else {
			// Can't use `res`, use an intermidate matrix.
			if(inter == GrB_NULL) {
				GrB_Matrix_nrows(&nrows, res);
				GrB_Matrix_ncols(&ncols, res);
				GrB_Matrix_new(&inter, GrB_BOOL, nrows, ncols);
			}
			b = _AlgebraicExpression_Eval(right, inter);
		}

		// Perform addition.
		if(GrB_eWiseAdd_Matrix_Semiring(res, GrB_NULL, GrB_NULL, GxB_LAND_LOR_BOOL, res, b,
										GrB_NULL) != GrB_SUCCESS) {
			printf("Failed adding operands, error:%s\n", GrB_error());
			assert(false);
		}
	}

	if(inter != GrB_NULL) GrB_Matrix_free(&inter);
	return res;
}

GrB_Matrix _AlgebraicExpression_Eval(const AlgebraicExpression *exp, GrB_Matrix res) {
	assert(exp);

	// Perform operation.
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
			assert("Unknown algebraic expression operation" && false);
		}
	case AL_OPERAND:
		res = exp->operand.matrix;
		break;
	default:
		assert("Unknow algebraic expression node type" && false);
	}

	return res;
}

void AlgebraicExpression_Eval(const AlgebraicExpression *exp, GrB_Matrix res) {
	assert(exp && exp->type == AL_OPERATION);
	_AlgebraicExpression_Eval(exp, res);
}
