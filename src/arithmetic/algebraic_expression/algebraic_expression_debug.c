/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/
#include "../algebraic_expression.h"
#include "utils.h"
#include "../../RG.h"
#include "../../util/rmalloc.h"
#include "../../util/arr.h"

//------------------------------------------------------------------------------
// AlgebraicExpression debugging utilities.
//------------------------------------------------------------------------------

static char keywords[5] = {'+', '*', '(', ')', 'T'};

bool _is_keyword(const char c) {
	for(int i = 0; i < sizeof(keywords); i++) {
		if(c == keywords[i]) return true;
	}
	return false;
}

AlgebraicExpression *_AlgebraicExpression_FromString
(
	const char **exp,   // String representation of expression.
	rax *matrices       // Map of matrices referred to in expression.
) {
	int i = 0;
	int len = 0;
	char *alias;
	const char *operand;
	GrB_Matrix m;
	AlgebraicExpression *op;
	AlgebraicExpression *rhs;
	AlgebraicExpression *root = NULL;

	while(*exp[0] != '\0') {
		char c = (*exp)[0];

		switch(c) {
		case '+':
			*exp = *exp + 1; // Advance.
			op = AlgebraicExpression_NewOperation(AL_EXP_ADD);
			rhs = _AlgebraicExpression_FromString(exp, matrices);
			AlgebraicExpression_AddChild(op, root);
			AlgebraicExpression_AddChild(op, rhs);
			root = op;
			break;
		case '*':
			*exp = *exp + 1; // Advance.
			op = AlgebraicExpression_NewOperation(AL_EXP_MUL);
			rhs = _AlgebraicExpression_FromString(exp, matrices);
			AlgebraicExpression_AddChild(op, root);
			AlgebraicExpression_AddChild(op, rhs);
			root = op;
			break;
		case '(':
			*exp = *exp + 1; // Advance.
			// Beginning of sub expression.
			return _AlgebraicExpression_FromString(exp, matrices);
			break;
		case ')':
			*exp = *exp + 1; // Advance.
			// End of sub expression.
			return root;
			break;
		case 'T':
			*exp = *exp + 1; // Advance.
			root = _AlgebraicExpression_FromString(exp, matrices);
			AlgebraicExpression_Transpose(&root);
			break;
		default:
			// Operand, consume operand name.
			operand = *exp;
			len = strlen(operand);
			while(i < len && !_is_keyword(operand[i])) i++;

			alias = (char *)malloc(sizeof(char) * i + 1);
			memcpy(alias, *exp, i);
			alias[i] = '\0';
			*exp = *exp + i; // Advance.

			m = GrB_NULL;
			if(matrices) {
				m = (GrB_Matrix)raxFind(matrices, (unsigned char *)alias, strlen(alias));
				ASSERT(m != raxNotFound && "Missing matrix");
			}
			root = AlgebraicExpression_NewOperand(m, false, alias, alias, NULL, NULL, AlgExpReference_NewEmpty());

			// Clear
			i = 0;
			len = 0;
			break;
		}
	}

	return root;
}

AlgebraicExpression *AlgebraicExpression_FromString
(
	const char *exp,    // String representation of expression.
	rax *matrices       // Map of matrices referred to in expression.
) {
	AlgebraicExpression *root = _AlgebraicExpression_FromString(&exp, matrices);
	AlgebraicExpression_Optimize(&root);
	return root;
}

static void _AlgebraicExpression_PrintTree
(
	const AlgebraicExpression *exp,
	uint ident
) {
	uint child_count = 0;
	const char *alias = NULL;

	printf("%*s", ident * 4, "");
	switch(exp->type) {
	case AL_OPERATION:
		child_count = AlgebraicExpression_ChildCount(exp);
		switch(exp->operation.op) {
		case AL_EXP_ADD:
			printf("+\n");
			for(uint i = 0; i < child_count; i++) {
				_AlgebraicExpression_PrintTree(exp->operation.children[i], ident + 1);
			}
			break;
		case AL_EXP_MUL:
			printf("*\n");
			for(uint i = 0; i < child_count; i++) {
				_AlgebraicExpression_PrintTree(exp->operation.children[i], ident + 1);
			}
			break;
		case AL_EXP_TRANSPOSE:
			printf("Transpose\n");
			for(uint i = 0; i < child_count; i++) {
				_AlgebraicExpression_PrintTree(exp->operation.children[i], ident + 1);
			}
			break;
		default:
			ASSERT("Unknown algebraic expression operation");
			break;
		}
		break;
	case AL_OPERAND:
		if(exp->operand.edge) alias = exp->operand.edge;
		else alias = exp->operand.src;
		printf("%s\n", alias);
	default:
		ASSERT("Unknown algebraic expression node type");
		break;
	}
}

void AlgebraicExpression_PrintTree
(
	const AlgebraicExpression *exp  // Root node.
) {
	_AlgebraicExpression_PrintTree(exp, 0);
}

void AlgebraicExpression_Print
(
	const AlgebraicExpression *exp  // Root node.
) {
	ASSERT(exp);
	const char *alias;

	switch(exp->type) {
	case AL_OPERATION:
		switch(exp->operation.op) {
		case AL_EXP_ADD:
			printf("(");
			// Print add first operand.
			AlgebraicExpression_Print(FIRST_CHILD(exp));
			// Expecting at least 2 operands, concat using '+'.
			for(uint i = 1; i < AlgebraicExpression_ChildCount(exp); i++) {
				printf(" + ");
				AlgebraicExpression_Print(CHILD_AT(exp, i));
			}
			printf(")");
			break;
		case AL_EXP_MUL:
			// Print add first operand.
			AlgebraicExpression_Print(FIRST_CHILD(exp));
			// Expecting at least 2 operands, concat using '*'.
			for(uint i = 1; i < AlgebraicExpression_ChildCount(exp); i++) {
				printf(" * ");
				AlgebraicExpression_Print(CHILD_AT(exp, i));
			}
			break;
		case AL_EXP_TRANSPOSE:
			// Expecting a single child.
			ASSERT(AlgebraicExpression_ChildCount(exp) == 1);
			printf("Transpose(");
			AlgebraicExpression_Print(FIRST_CHILD(exp));
			printf(")");
			break;
		default:
			ASSERT("Unknown algebraic expression operation");
			break;
		}
		break;
	case AL_OPERAND:
		if(exp->operand.edge) alias = exp->operand.edge;
		else if(exp->operand.label) alias = exp->operand.label;
		else alias = exp->operand.src;
		printf("%s", alias);
	default:
		ASSERT("Unknown algebraic expression node type");
		break;
	}
}

void _AlgebraicExpression_ToString
(
	const AlgebraicExpression *exp, // Root node.
	char *buff
) {
	ASSERT(exp);
	const char *alias;

	switch(exp->type) {
	case AL_OPERATION:
		switch(exp->operation.op) {
		case AL_EXP_ADD:
			sprintf(buff + strlen(buff), "(");
			// Print add first operand.
			_AlgebraicExpression_ToString(FIRST_CHILD(exp), buff);
			// Expecting at least 2 operands, concat using '+'.
			for(uint i = 1; i < AlgebraicExpression_ChildCount(exp); i++) {
				sprintf(buff + strlen(buff), " + ");
				_AlgebraicExpression_ToString(CHILD_AT(exp, i), buff);
			}
			sprintf(buff + strlen(buff), ")");
			break;
		case AL_EXP_MUL:
			// Print add first operand.
			_AlgebraicExpression_ToString(FIRST_CHILD(exp), buff);
			// Expecting at least 2 operands, concat using '*'.
			for(uint i = 1; i < AlgebraicExpression_ChildCount(exp); i++) {
				sprintf(buff + strlen(buff), " * ");
				_AlgebraicExpression_ToString(CHILD_AT(exp, i), buff);
			}
			break;
		case AL_EXP_TRANSPOSE:
			// Expecting a single child.
			ASSERT(AlgebraicExpression_ChildCount(exp) == 1);
			sprintf(buff + strlen(buff), "Transpose(");
			_AlgebraicExpression_ToString(FIRST_CHILD(exp), buff);
			sprintf(buff + strlen(buff), ")");
			break;
		default:
			ASSERT("Unknown algebraic expression operation");
			break;
		}
		break;
	case AL_OPERAND:
		if(exp->operand.edge) alias = exp->operand.edge;
		else alias = exp->operand.src;
		sprintf(buff + strlen(buff), "%s", alias);
	default:
		ASSERT("Unknown algebraic expression node type");
		break;
	}
}

char *AlgebraicExpression_ToString
(
	const AlgebraicExpression *exp  // Root node.
) {
	char *buff = rm_calloc(1024, sizeof(char));
	_AlgebraicExpression_ToString(exp, buff);
	return buff;
}

void _AlgebraicExpression_ToStringDebug
        (
                const AlgebraicExpression *exp, // Root node.
                char *buff
        ) {
    assert(exp);
    const char *label;

    switch(exp->type) {
        case AL_OPERATION:
            switch(exp->operation.op) {
                case AL_EXP_ADD:
                    sprintf(buff + strlen(buff), "add(");
                    // Print add first operand.
                    _AlgebraicExpression_ToStringDebug(FIRST_CHILD(exp), buff);
                    // Expecting at least 2 operands, concat using '+'.
                    for(uint i = 1; i < AlgebraicExpression_ChildCount(exp); i++) {
                        sprintf(buff + strlen(buff), ", ");
                        _AlgebraicExpression_ToStringDebug(CHILD_AT(exp, i), buff);
                    }
                    sprintf(buff + strlen(buff), ")");
                    break;
                case AL_EXP_MUL:
                    // Print add first operand.
                    sprintf(buff + strlen(buff), "mul(");
                    _AlgebraicExpression_ToStringDebug(FIRST_CHILD(exp), buff);
                    // Expecting at least 2 operands, concat using '*'.
                    for(uint i = 1; i < AlgebraicExpression_ChildCount(exp); i++) {
                        sprintf(buff + strlen(buff), ", ");
                        _AlgebraicExpression_ToStringDebug(CHILD_AT(exp, i), buff);
                    }
                    sprintf(buff + strlen(buff), ")");
                    break;
                case AL_EXP_TRANSPOSE:
                    // Expecting a single child.
                    assert(AlgebraicExpression_ChildCount(exp) == 1);
                    sprintf(buff + strlen(buff), "Transpose(");
                    _AlgebraicExpression_ToStringDebug(FIRST_CHILD(exp), buff);
                    sprintf(buff + strlen(buff), ")");
                    break;
                default:
                    assert("Unknown algebraic expression operation");
                    break;
            }
            break;
        case AL_OPERAND:
            if(AlgebraicExpression_OperandIsReference(exp))
                sprintf(buff + strlen(buff), "%s-%s:~%s:%d-%s",
						exp->operand.src,
						exp->operand.edge,
						exp->operand.reference.name,
						exp->operand.reference.transposed,
						exp->operand.dest);
            else
                sprintf(buff + strlen(buff), "%s-%s:%s-%s", exp->operand.src, exp->operand.edge, exp->operand.label, exp->operand.dest);
        default:
            assert("Unknown algebraic expression node type");
            break;
    }
}

char *AlgebraicExpression_ToStringDebug
(
	const AlgebraicExpression *exp  // Root node.
) {
    char *buff = rm_calloc(1024, sizeof(char));
    _AlgebraicExpression_ToStringDebug(exp, buff);
    return buff;
}

void _AlgebraicExpression_TotalShow(
	const AlgebraicExpression *exp  // Root node.
) {
	printf("AE: %s\n", AlgebraicExpression_ToStringDebug(exp));
	switch(exp->type) {
		case AL_OPERATION:
			for (int i = 0; i < array_len(exp->operation.children); ++i) {
				_AlgebraicExpression_TotalShow(exp->operation.children[i]);
			}
			break;
		case AL_OPERAND:
			printf("%s (%p):\n", AlgebraicExpression_ToStringDebug(exp), exp->operand.matrix);
			if (exp->operand.matrix == IDENTITY_MATRIX) {
				printf("Identity\n");
			} else {
				GxB_Matrix_fprint(exp->operand.matrix, "some", GxB_COMPLETE, stdout);
			}
			break;
	}
}
