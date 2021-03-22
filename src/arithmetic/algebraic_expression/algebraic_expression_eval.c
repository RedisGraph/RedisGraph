/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "utils.h"
#include "../../query_ctx.h"
#include "../algebraic_expression.h"

// Forward declarations
GrB_Matrix _AlgebraicExpression_Eval(const AlgebraicExpression *exp, GrB_Matrix res, PathPatternCtx *pathCtx);

// dest += {(j, j): (i, j) in left}
static void _Add_Sources(GrB_Matrix left, GrB_Matrix dest, bool left_transposed) {
#ifdef DPP
	printf("_Add_Sources\n");
	printf("left:\n");
	GxB_print(left, GxB_COMPLETE);
#endif

	GrB_Index n;

	GrB_Descriptor reduce_desc;
	GrB_Descriptor_new(&reduce_desc);

	if (left_transposed) {
		GrB_Matrix_nrows(&n, left);
	} else {
		GrB_Matrix_ncols(&n, left);
		GrB_Descriptor_set(reduce_desc, GrB_INP0, GrB_TRAN);
	}

	GrB_Vector sources;
	GrB_Vector_new(&sources, GrB_BOOL, n);
	GrB_reduce(sources, NULL, NULL, GrB_LOR, left, reduce_desc);

#ifdef DPP
	printf("reduced vector\n");
	GxB_print(sources, GxB_COMPLETE);
#endif

	GrB_Index nvals;
	GrB_Vector_nvals(&nvals, sources);

	GrB_Index *index = malloc(sizeof(GrB_Index) * nvals);
	GrB_Vector_extractTuples_BOOL(index, NULL, &nvals, sources);
	for (GrB_Index i = 0; i < nvals; ++i) {
		GrB_Matrix_setElement_BOOL(dest, true, index[i], index[i]);
	}

	GrB_Vector_free(&sources);
	GrB_Descriptor_free(&reduce_desc);
	free(index);
}

static GrB_Matrix _Eval_Transpose
(
	const AlgebraicExpression *exp,
	GrB_Matrix res,
	PathPatternCtx *pathCtx
) {
	// This function is currently unused.
	ASSERT(exp && AlgebraicExpression_ChildCount(exp) == 1);

	AlgebraicExpression *child = FIRST_CHILD(exp);
	ASSERT(child->type == AL_OPERAND);
	GrB_Info info = GrB_transpose(res, GrB_NULL, GrB_NULL, child->operand.matrix, GrB_NULL);
	ASSERT(info == GrB_SUCCESS);
	return res;
}

static GrB_Matrix _Eval_Add(const AlgebraicExpression *exp, GrB_Matrix res, PathPatternCtx *pathCtx) {
	ASSERT(exp && AlgebraicExpression_ChildCount(exp) > 1);

	GrB_Info info;
	UNUSED(info);
	GrB_Index nrows;                // Number of rows of operand.
	GrB_Index ncols;                // Number of columns of operand.
	bool res_in_use = false;        // Can we use `res` for intermediate evaluation.
	GrB_Matrix a = GrB_NULL;        // Left operand.
	GrB_Matrix b = GrB_NULL;        // Right operand.
	GrB_Matrix inter = GrB_NULL;    // Intermediate matrix.
	GrB_Descriptor desc = GrB_NULL; // Descriptor used for transposing operands (currently unused).

	// Get left and right operands.
	AlgebraicExpression *left = CHILD_AT(exp, 0);
	AlgebraicExpression *right = CHILD_AT(exp, 1);

	/* If left operand is a matrix, simply get it.
	 * Otherwise evaluate left hand side using `res` to store LHS value. */
	if(left->type == AL_OPERAND) {
		a = left->operand.matrix;
	} else {
		if(left->operation.op == AL_EXP_TRANSPOSE) {
			ASSERT(AlgebraicExpression_ChildCount(left) == 1);
			a = left->operation.children[0]->operand.matrix;
			if(desc == GrB_NULL) GrB_Descriptor_new(&desc);
			GrB_Descriptor_set(desc, GrB_INP0, GrB_TRAN);
		} else {
			a = _AlgebraicExpression_Eval(left, res, pathCtx);
			res_in_use = true;
		}
	}

	/* If right operand is a matrix, simply get it.
	 * Otherwise evaluate right hand side using `res` if free or create an additional matrix to store RHS value. */
	if(right->type == AL_OPERAND) {
		b = right->operand.matrix;
	} else {
		if(right->operation.op == AL_EXP_TRANSPOSE) {
			ASSERT(AlgebraicExpression_ChildCount(right) == 1);
			b = right->operation.children[0]->operand.matrix;
			if(desc == GrB_NULL) GrB_Descriptor_new(&desc);
			GrB_Descriptor_set(desc, GrB_INP1, GrB_TRAN);
		} else if(res_in_use) {
			// `res` is in use, create an additional matrix.
			GrB_Matrix_nrows(&nrows, a);
			GrB_Matrix_ncols(&ncols, a);
			info = GrB_Matrix_new(&inter, GrB_BOOL, nrows, ncols);
			ASSERT(info == GrB_SUCCESS);
			b = _AlgebraicExpression_Eval(right, inter, pathCtx);
		} else {
			// `res` is not used just yet, use it for RHS evaluation.
			b = _AlgebraicExpression_Eval(right, res, pathCtx);
		}
	}

	// Perform addition.
	info = GrB_eWiseAdd(res, GrB_NULL, GrB_NULL, GxB_ANY_PAIR_BOOL, a, b, desc);
	ASSERT(info == GrB_SUCCESS);

	// Reset descriptor if non-null.
	if(desc != GrB_NULL) GrB_Descriptor_set(desc, GrB_INP0, GxB_DEFAULT);

	uint child_count = AlgebraicExpression_ChildCount(exp);
	// Expression has more than 2 operands, e.g. A+B+C...
	for(uint i = 2; i < child_count; i++) {
		// Reset descriptor if non-null.
		if(desc != GrB_NULL) GrB_Descriptor_set(desc, GrB_INP1, GxB_DEFAULT);
		right = CHILD_AT(exp, i);

		if(right->type == AL_OPERAND) {
			b = right->operand.matrix;
		} else {
			if(right->operation.op == AL_EXP_TRANSPOSE) {
				ASSERT(AlgebraicExpression_ChildCount(right) == 1);
				b = right->operation.children[0]->operand.matrix;
				if(desc == GrB_NULL) GrB_Descriptor_new(&desc);
				GrB_Descriptor_set(desc, GrB_INP1, GrB_TRAN);
			} else {
				// 'right' represents either + or * operation.
				if(inter == GrB_NULL) {
					// Can't use `res`, use an intermidate matrix.
					GrB_Matrix_nrows(&nrows, res);
					GrB_Matrix_ncols(&ncols, res);
					info = GrB_Matrix_new(&inter, GrB_BOOL, nrows, ncols);
					ASSERT(info == GrB_SUCCESS);
				}
				b = _AlgebraicExpression_Eval(right, inter, pathCtx);
			}
		}

		// Perform addition.
		info = GrB_eWiseAdd(res, GrB_NULL, GrB_NULL, GxB_ANY_PAIR_BOOL, res, b, GrB_NULL);
		ASSERT(info == GrB_SUCCESS);
	}

	if(inter != GrB_NULL) GrB_Matrix_free(&inter);
	if(desc != GrB_NULL) GrB_free(&desc);
	return res;
}

static GrB_Matrix _Eval_Mul(const AlgebraicExpression *exp, GrB_Matrix res, PathPatternCtx *pathCtx) {
	ASSERT(exp &&
		   AlgebraicExpression_ChildCount(exp) > 1 &&
		   AlgebraicExpression_OperationCount(exp, AL_EXP_MUL) == 1);

	GrB_Info info;
	UNUSED(info);
	GrB_Matrix A;
	bool A_trans = false;

	GrB_Matrix B;
	bool B_trans = false;

	GrB_Index nvals;
	GrB_Descriptor desc = GrB_NULL;
	AlgebraicExpression *left = CHILD_AT(exp, 0);
	AlgebraicExpression *right = CHILD_AT(exp, 1);

	if(left->type == AL_OPERATION) {
		ASSERT(left->operation.op == AL_EXP_TRANSPOSE);
		ASSERT(AlgebraicExpression_ChildCount(left) == 1);
		if(desc == GrB_NULL) GrB_Descriptor_new(&desc);
		GrB_Descriptor_set(desc, GrB_INP0, GrB_TRAN);
		left = CHILD_AT(left, 0);
		A_trans = true;
	}
	A = left->operand.matrix;

	if(right->type == AL_OPERATION) {
		ASSERT(right->operation.op == AL_EXP_TRANSPOSE);
		ASSERT(AlgebraicExpression_ChildCount(right) == 1);
		if(desc == GrB_NULL) GrB_Descriptor_new(&desc);
		GrB_Descriptor_set(desc, GrB_INP1, GrB_TRAN);
		right = CHILD_AT(right, 0);
		B_trans = true;
	}
	B = right->operand.matrix;

	if(B == IDENTITY_MATRIX) {
		// Reset descriptor, as the identity matrix does not need to be transposed.
		if(desc != GrB_NULL) GrB_Descriptor_set(desc, GrB_INP1, GxB_DEFAULT);
		// B is the identity matrix, Perform A * I.
		info = GrB_Matrix_apply(res, GrB_NULL, GrB_NULL, GrB_IDENTITY_BOOL, A, desc);
		ASSERT(info == GrB_SUCCESS);
	} else {
		// If B is matrix of named path pattern, add sources
		if (AlgebraicExpression_OperandIsReference(right)) {
			if (B_trans) {
				fprintf(stderr, "don`t support trans named path pattern %s\n", right->operand.reference.name);
				ASSERT(false);
			}
			ASSERT(pathCtx != NULL && "Path pattern context is NULL, "
							          "but there is a reference inside algebraic expression");
			PathPattern *pattern = PathPatternCtx_GetPathPattern(pathCtx, right->operand.reference);
			_Add_Sources(A, pattern->src, A_trans);
		}

		// Perform multiplication.
		info = GrB_mxm(res, GrB_NULL, GrB_NULL, GxB_ANY_PAIR_BOOL, A, B, desc);
		ASSERT(info == GrB_SUCCESS);
	}

	GrB_wait(&res);

	// Reset descriptor if non-null.
	if(desc != GrB_NULL) GrB_Descriptor_set(desc, GrB_INP0, GxB_DEFAULT);

	uint child_count = AlgebraicExpression_ChildCount(exp);
	for(uint i = 2; i < child_count; i++) {
		// Reset descriptor if non-null.
		if(desc != GrB_NULL) GrB_Descriptor_set(desc, GrB_INP1, GxB_DEFAULT);

		right = CHILD_AT(exp, i);
		if(right->type == AL_OPERATION) {
			ASSERT(right->operation.op == AL_EXP_TRANSPOSE);
			ASSERT(AlgebraicExpression_ChildCount(right) == 1);
			if(desc == GrB_NULL) GrB_Descriptor_new(&desc);
			GrB_Descriptor_set(desc, GrB_INP1, GrB_TRAN);
			right = CHILD_AT(right, 0);
			B_trans = true;
		}
		B = right->operand.matrix;

		if(B != IDENTITY_MATRIX) {
			if (right->operand.reference.name) {
				if (B_trans) {
					fprintf(stderr,
			 		"Named path patterns should be first "
					 	"processed to avoid transpositions %s\n", right->operand.reference.name);
					ASSERT(false);
				}
				ASSERT(pathCtx != NULL && "Path pattern context is NULL, "
										  "but there is a reference inside algebraic expression");
				PathPattern *pattern = PathPatternCtx_GetPathPattern(pathCtx, right->operand.reference);
				_Add_Sources(res, pattern->src, false);
			}

			// Reset descriptor, as the identity matrix does not need to be transposed.
			if(desc != GrB_NULL) GrB_Descriptor_set(desc, GrB_INP1, GxB_DEFAULT);
			// Perform multiplication.

			info = GrB_mxm(res, GrB_NULL, GrB_NULL, GxB_ANY_PAIR_BOOL, res, B, desc);
			ASSERT(info == GrB_SUCCESS);
		}
		GrB_Matrix_nvals(&nvals, res);
		if(nvals == 0) break;
	}

	if(desc != GrB_NULL) GrB_free(&desc);

	return res;
}

GrB_Matrix _AlgebraicExpression_Eval(const AlgebraicExpression *exp, GrB_Matrix res, PathPatternCtx *pathCtx) {
	ASSERT(exp);

	// Perform operation.
	switch(exp->type) {
	case AL_OPERATION:
		switch(exp->operation.op) {
		case AL_EXP_MUL:
			res = _Eval_Mul(exp, res, pathCtx);
			break;

		case AL_EXP_ADD:
			res = _Eval_Add(exp, res, pathCtx);
			break;

		case AL_EXP_TRANSPOSE:
			res = _Eval_Transpose(exp, res, pathCtx);
			break;

		default:
			ASSERT("Unknown algebraic expression operation" && false);
		}
		break;
	case AL_OPERAND:
		res = exp->operand.matrix;
		break;
	default:
		ASSERT("Unknown algebraic expression node type" && false);
	}

	return res;
}

void AlgebraicExpression_Eval(const AlgebraicExpression *exp, GrB_Matrix res, PathPatternCtx *pathCtx) {
	ASSERT(exp && exp->type == AL_OPERATION);
	_AlgebraicExpression_Eval(exp, res, pathCtx);
}

