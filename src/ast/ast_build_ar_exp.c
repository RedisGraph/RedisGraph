/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "ast_build_ar_exp.h"
#include "../util/rmalloc.h"
#include "../arithmetic/funcs.h"
#include <assert.h>

// Forward declaration
static AR_ExpNode *_AR_EXP_FromExpression(const cypher_astnode_t *expr);

static const char *_ASTOpToString(AST_Operator op) {
	// TODO: switch to a table, tbl[op] = string.
	switch(op) {
	case OP_PLUS:
		return "ADD";
	case OP_MINUS:
		return "SUB";
	case OP_MULT:
		return "MUL";
	case OP_DIV:
		return "DIV";
	case OP_CONTAINS:
		return "CONTAINS";
	case OP_STARTSWITH:
		return "STARTS WITH";
	case OP_ENDSWITH:
		return "ENDS WITH";
	case OP_AND:
		return "AND";
	case OP_OR:
		return "OR";
	case OP_XOR:
		return "XOR";
	case OP_NOT:
		return "NOT";
	case OP_GT:
		return "GT";
	case OP_GE:
		return "GE";
	case OP_LT:
		return "LT";
	case OP_LE:
		return "LE";
	case OP_EQUAL:
		return "EQ";
	case OP_NEQUAL:
		return "NEQ";
	case OP_MOD:
		return "MOD";
	case OP_POW:
		return "POW";
	case OP_IN:
		return "IN";
	case OP_IS_NULL:
		return "IS NULL";
	case OP_IS_NOT_NULL:
		return "IS NOT NULL";
	default:
		assert(false && "Unhandled operator was specified in query");
		return NULL;
	}
}

static AR_ExpNode *AR_EXP_NewOpNodeFromAST(AST_Operator op, uint child_count) {
	const char *func_name = _ASTOpToString(op);
	return AR_EXP_NewOpNode(func_name, child_count);
}

static AR_ExpNode *_AR_EXP_FromApplyExpression(const cypher_astnode_t *expr) {
	// TODO handle CYPHER_AST_APPLY_ALL_OPERATOR
	AR_ExpNode *op;
	const cypher_astnode_t *func_node = cypher_ast_apply_operator_get_func_name(expr);
	const char *func_name = cypher_ast_function_name_get_value(func_node);
	unsigned int arg_count = cypher_ast_apply_operator_narguments(expr);
	bool distinct = cypher_ast_apply_operator_get_distinct(expr);
	if(distinct) op = AR_EXP_NewDistinctOpNode(func_name, arg_count);
	else op = AR_EXP_NewOpNode(func_name, arg_count);

	for(unsigned int i = 0; i < arg_count; i ++) {
		const cypher_astnode_t *arg = cypher_ast_apply_operator_get_argument(expr, i);
		// Recursively convert arguments
		op->op.children[i] = _AR_EXP_FromExpression(arg);
	}
	return op;
}

static AR_ExpNode *_AR_EXP_FromIdentifierExpression(const cypher_astnode_t *expr) {
	// Identifier referencing another entity
	const char *alias = cypher_ast_identifier_get_name(expr);
	return AR_EXP_NewVariableOperandNode(alias, NULL);
}

static AR_ExpNode *_AR_EXP_FromPropertyExpression(const cypher_astnode_t *expr) {
	// Identifier and property pair
	// Extract the entity alias from the property. Currently, the embedded
	// expression should only refer to the IDENTIFIER type.
	const cypher_astnode_t *prop_expr = cypher_ast_property_operator_get_expression(expr);
	assert(cypher_astnode_type(prop_expr) == CYPHER_AST_IDENTIFIER);
	const char *alias = cypher_ast_identifier_get_name(prop_expr);

	// Extract the property name
	const cypher_astnode_t *prop_name_node = cypher_ast_property_operator_get_prop_name(expr);
	const char *prop_name = cypher_ast_prop_name_get_value(prop_name_node);

	return AR_EXP_NewVariableOperandNode(alias, prop_name);
}

static AR_ExpNode *_AR_EXP_FromIntegerExpression(const cypher_astnode_t *expr) {
	const char *value_str = cypher_ast_integer_get_valuestr(expr);
	char *endptr = NULL;
	int64_t l = strtol(value_str, &endptr, 0);
	assert(endptr[0] == 0);
	SIValue converted = SI_LongVal(l);
	return AR_EXP_NewConstOperandNode(converted);
}

static AR_ExpNode *_AR_EXP_FromFloatExpression(const cypher_astnode_t *expr) {
	const char *value_str = cypher_ast_float_get_valuestr(expr);
	char *endptr = NULL;
	double d = strtod(value_str, &endptr);
	assert(endptr[0] == 0);
	SIValue converted = SI_DoubleVal(d);
	return AR_EXP_NewConstOperandNode(converted);
}

static AR_ExpNode *_AR_EXP_FromStringExpression(const cypher_astnode_t *expr) {
	const char *value_str = cypher_ast_string_get_value(expr);
	SIValue converted = SI_ConstStringVal((char *)value_str);
	return AR_EXP_NewConstOperandNode(converted);
}

static AR_ExpNode *_AR_EXP_FromTrueExpression() {
	SIValue converted = SI_BoolVal(true);
	return AR_EXP_NewConstOperandNode(converted);
}

static AR_ExpNode *_AR_EXP_FromFalseExpression() {
	SIValue converted = SI_BoolVal(false);
	return AR_EXP_NewConstOperandNode(converted);
}

static AR_ExpNode *_AR_EXP_FromNullExpression() {
	SIValue converted = SI_NullVal();
	return AR_EXP_NewConstOperandNode(converted);
}

static AR_ExpNode *_AR_EXP_FromUnaryOpExpression(const cypher_astnode_t *expr) {
	AR_ExpNode *op = NULL;
	const cypher_astnode_t *arg = cypher_ast_unary_operator_get_argument(expr); // CYPHER_AST_EXPRESSION
	const cypher_operator_t *operator = cypher_ast_unary_operator_get_operator(expr);

	if(operator == CYPHER_OP_UNARY_MINUS) {
		// This expression can be something like -3 or -a.val
		// In the former case, we'll reduce the tree to a constant after building it fully.
		op = AR_EXP_NewOpNodeFromAST(OP_MULT, 2);
		op->op.children[0] = AR_EXP_NewConstOperandNode(SI_LongVal(-1));
		op->op.children[1] = _AR_EXP_FromExpression(arg);
	} else if(operator == CYPHER_OP_UNARY_PLUS) {
		/* This expression is something like +3 or +a.val.
		 * I think the + can always be safely ignored. */
		op = _AR_EXP_FromExpression(arg);
	} else if(operator == CYPHER_OP_NOT) {
		op = AR_EXP_NewOpNodeFromAST(OP_NOT, 1);
		op->op.children[0] = _AR_EXP_FromExpression(arg);
	} else if(operator == CYPHER_OP_IS_NULL) {
		op = AR_EXP_NewOpNodeFromAST(OP_IS_NULL, 1);
		op->op.children[0] = _AR_EXP_FromExpression(arg);
	} else if(operator == CYPHER_OP_IS_NOT_NULL) {
		op = AR_EXP_NewOpNodeFromAST(OP_IS_NOT_NULL, 1);
		op->op.children[0] = _AR_EXP_FromExpression(arg);
	} else {
		// No supported operator found.
		assert(false);
	}
	return op;
}

static AR_ExpNode *_AR_EXP_FromBinaryOpExpression(const cypher_astnode_t *expr) {
	const cypher_operator_t *operator = cypher_ast_binary_operator_get_operator(expr);
	AST_Operator operator_enum = AST_ConvertOperatorNode(operator);
	// Arguments are of type CYPHER_AST_EXPRESSION
	AR_ExpNode *op = AR_EXP_NewOpNodeFromAST(operator_enum, 2);
	const cypher_astnode_t *lhs_node = cypher_ast_binary_operator_get_argument1(expr);
	op->op.children[0] = _AR_EXP_FromExpression(lhs_node);
	const cypher_astnode_t *rhs_node = cypher_ast_binary_operator_get_argument2(expr);
	op->op.children[1] = _AR_EXP_FromExpression(rhs_node);
	return op;
}

static AR_ExpNode *_AR_EXP_FromComparisonExpression(const cypher_astnode_t *expr) {
	// 1 < 2 = 2 <= 4
	AR_ExpNode *op;
	uint length = cypher_ast_comparison_get_length(expr);
	if(length > 1) {
		op = AR_EXP_NewOpNodeFromAST(OP_AND, length);
		for(uint i = 0; i < length; i++) {
			const cypher_operator_t *operator = cypher_ast_comparison_get_operator(expr, i);
			AST_Operator operator_enum = AST_ConvertOperatorNode(operator);
			const cypher_astnode_t *lhs_node = cypher_ast_comparison_get_argument(expr, i);
			const cypher_astnode_t *rhs_node = cypher_ast_comparison_get_argument(expr, i + 1);

			AR_ExpNode *inner_op = AR_EXP_NewOpNodeFromAST(operator_enum, 2);
			inner_op->op.children[0] = _AR_EXP_FromExpression(lhs_node);
			inner_op->op.children[1] = _AR_EXP_FromExpression(rhs_node);
			op->op.children[i] = inner_op;
		}
	} else {
		const cypher_operator_t *operator = cypher_ast_comparison_get_operator(expr, 0);
		AST_Operator operator_enum = AST_ConvertOperatorNode(operator);
		op = AR_EXP_NewOpNodeFromAST(operator_enum, 2);
		const cypher_astnode_t *lhs_node = cypher_ast_comparison_get_argument(expr, 0);
		const cypher_astnode_t *rhs_node = cypher_ast_comparison_get_argument(expr, 1);
		op->op.children[0] = _AR_EXP_FromExpression(lhs_node);
		op->op.children[1] = _AR_EXP_FromExpression(rhs_node);
	}
	return op;
}

static AR_ExpNode *_AR_EXP_FromCaseExpression(const cypher_astnode_t *expr) {
	//Determin number of child expressions:
	unsigned int arg_count;
	const cypher_astnode_t *expression = cypher_ast_case_get_expression(expr);
	unsigned int alternatives = cypher_ast_case_nalternatives(expr);

	/* Simple form: 2 * alternatives + default
	 * Generic form: 2 * alternatives + default */
	if(expression) arg_count = 1 + 2 * alternatives + 1;
	else arg_count = 2 * alternatives + 1;

	// Create Expression and child expressions
	AR_ExpNode *op = AR_EXP_NewOpNode("case", arg_count);

	// Value to compare against
	int offset = 0;
	if(expression != NULL) {
		op->op.children[offset++] = _AR_EXP_FromExpression(expression);
	}

	// Alternatives
	for(uint i = 0; i < alternatives; i++) {
		const cypher_astnode_t *predicate = cypher_ast_case_get_predicate(expr, i);
		op->op.children[offset++] = _AR_EXP_FromExpression(predicate);
		const cypher_astnode_t *value = cypher_ast_case_get_value(expr, i);
		op->op.children[offset++] = _AR_EXP_FromExpression(value);
	}

	// Default value.
	const cypher_astnode_t *deflt = cypher_ast_case_get_default(expr);
	if(deflt == NULL) {
		// Default not specified, use NULL.
		op->op.children[offset] = AR_EXP_NewConstOperandNode(SI_NullVal());
	} else {
		op->op.children[offset] = _AR_EXP_FromExpression(deflt);
	}

	return op;
}

static AR_ExpNode *_AR_ExpFromCollectionExpression(const cypher_astnode_t *expr) {
	uint expCount = cypher_ast_collection_length(expr);
	AR_ExpNode *op = AR_EXP_NewOpNode("tolist", expCount);
	for(uint i = 0; i < expCount; i ++) {
		const cypher_astnode_t *exp_node = cypher_ast_collection_get(expr, i);
		op->op.children[i] = AR_EXP_FromExpression(exp_node);
	}
	return op;
}

static AR_ExpNode *_AR_ExpFromSubscriptExpression(const cypher_astnode_t *expr) {
	AR_ExpNode *op = AR_EXP_NewOpNode("subscript", 2);
	const cypher_astnode_t *exp_node = cypher_ast_subscript_operator_get_expression(expr);
	op->op.children[0] = AR_EXP_FromExpression(exp_node);
	const cypher_astnode_t *subscript_node = cypher_ast_subscript_operator_get_subscript(expr);
	op->op.children[1] = AR_EXP_FromExpression(subscript_node);
	return op;
}

static AR_ExpNode *_AR_ExpFromSliceExpression(const cypher_astnode_t *expr) {
	AR_ExpNode *op = AR_EXP_NewOpNode("slice", 3);
	const cypher_astnode_t *exp_node = cypher_ast_slice_operator_get_expression(expr);
	const cypher_astnode_t *start_node = cypher_ast_slice_operator_get_start(expr);
	const cypher_astnode_t *end_node = cypher_ast_slice_operator_get_end(expr);

	op->op.children[0] = AR_EXP_FromExpression(exp_node);

	if(start_node) op->op.children[1] = AR_EXP_FromExpression(start_node);
	else op->op.children[1] = AR_EXP_NewConstOperandNode(SI_LongVal(0));

	if(end_node) op->op.children[2] = AR_EXP_FromExpression(end_node);
	else op->op.children[2] = AR_EXP_NewConstOperandNode(SI_LongVal(INT32_MAX));

	return op;
}

static AR_ExpNode *_AR_EXP_FromExpression(const cypher_astnode_t *expr) {
	const cypher_astnode_type_t type = cypher_astnode_type(expr);

	/* Function invocations */
	if(type == CYPHER_AST_APPLY_OPERATOR || type == CYPHER_AST_APPLY_ALL_OPERATOR) {
		return _AR_EXP_FromApplyExpression(expr);
		/* Variables (full nodes and edges, UNWIND artifacts */
	} else if(type == CYPHER_AST_IDENTIFIER) {
		return _AR_EXP_FromIdentifierExpression(expr);
		/* Entity-property pair */
	} else if(type == CYPHER_AST_PROPERTY_OPERATOR) {
		return _AR_EXP_FromPropertyExpression(expr);
		/* SIValue constant types */
	} else if(type == CYPHER_AST_INTEGER) {
		return _AR_EXP_FromIntegerExpression(expr);
	} else if(type == CYPHER_AST_FLOAT) {
		return _AR_EXP_FromFloatExpression(expr);
	} else if(type == CYPHER_AST_STRING) {
		return _AR_EXP_FromStringExpression(expr);
	} else if(type == CYPHER_AST_TRUE) {
		return _AR_EXP_FromTrueExpression();
	} else if(type == CYPHER_AST_FALSE) {
		return _AR_EXP_FromFalseExpression();
	} else if(type == CYPHER_AST_NULL) {
		return _AR_EXP_FromNullExpression();
		/* Handling for unary operators (-5, +a.val) */
	} else if(type == CYPHER_AST_UNARY_OPERATOR) {
		return _AR_EXP_FromUnaryOpExpression(expr);
	} else if(type == CYPHER_AST_BINARY_OPERATOR) {
		return _AR_EXP_FromBinaryOpExpression(expr);
	} else if(type == CYPHER_AST_COMPARISON) {
		return _AR_EXP_FromComparisonExpression(expr);
	} else if(type == CYPHER_AST_CASE) {
		return _AR_EXP_FromCaseExpression(expr);
	} else if(type == CYPHER_AST_COLLECTION) {
		return _AR_ExpFromCollectionExpression(expr);
	} else if(type == CYPHER_AST_SUBSCRIPT_OPERATOR) {
		return _AR_ExpFromSubscriptExpression(expr);
	} else if(type == CYPHER_AST_SLICE_OPERATOR) {
		return _AR_ExpFromSliceExpression(expr);
	} else {
		/*
		   Unhandled types:
		   CYPHER_AST_LABELS_OPERATOR
		   CYPHER_AST_LIST_COMPREHENSION
		   CYPHER_AST_MAP
		   CYPHER_AST_MAP_PROJECTION
		   CYPHER_AST_PARAMETER
		   CYPHER_AST_PATTERN_COMPREHENSION
		   CYPHER_AST_REDUCE
		*/
		printf("Encountered unhandled type '%s'\n", cypher_astnode_typestr(type));
		assert(false);
	}

	assert(false);
	return NULL;
}

AR_ExpNode *AR_EXP_FromExpression(const cypher_astnode_t *expr) {
	AR_ExpNode *root = _AR_EXP_FromExpression(expr);
	AR_EXP_ReduceToScalar(&root);
	return root;
}

