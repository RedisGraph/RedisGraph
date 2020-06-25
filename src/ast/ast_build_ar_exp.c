/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "ast_build_ar_exp.h"
#include "../query_ctx.h"
#include "../util/rmalloc.h"
#include "../arithmetic/funcs.h"
#include <assert.h>
#include "../query_ctx.h"

// Forward declaration
static AR_ExpNode *_AR_EXP_FromExpression(const cypher_astnode_t *expr);

static bool __AR_EXP_ContainsNestedAgg(const AR_ExpNode *root, bool in_agg) {
	// Is this an aggregation node?
	bool agg_node = (root->type == AR_EXP_OP && root->op.type == AR_OP_AGGREGATE);
	// Aggregation node nested within another aggregation node.
	if(agg_node && in_agg) return true;

	if(root->type == AR_EXP_OP) {
		// Scan child nodes.
		for(int i = 0; i < root->op.child_count; i++) {
			AR_ExpNode *child = root->op.children[i];
			// Cary on `in_agg`.
			if(__AR_EXP_ContainsNestedAgg(child, agg_node | in_agg)) return true;
		}
	}

	return false;
}

/* Return true if expression contains nested calls to aggregation functions
 * e.g. MAX(MIN(1)) */
static bool _AR_EXP_ContainsNestedAgg(const AR_ExpNode *exp) {
	bool in_agg = false;
	return __AR_EXP_ContainsNestedAgg(exp, in_agg);
}

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

static AR_ExpNode *_AR_EXP_FromApplyAllExpression(const cypher_astnode_t *expr) {
	// ApplyAll operators use accessors similar to normal Apply operators with the exception
	// that they have no argument accessors - by definition, they have one argument (all/STAR).
	const cypher_astnode_t *func_node = cypher_ast_apply_all_operator_get_func_name(expr);
	const char *func_name = cypher_ast_function_name_get_value(func_node);

	AR_ExpNode *op = AR_EXP_NewOpNode(func_name, 1);

	// Introduce a fake child constant so that the function always operates on something.
	op->op.children[0] = AR_EXP_NewConstOperandNode(SI_BoolVal(1));

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
	if(endptr[0] != 0) {
		// Failed to convert integer value; set compile-time error to be raised later.
		QueryCtx_SetError("Invalid numeric value '%s'", value_str);
		return AR_EXP_NewConstOperandNode(SI_NullVal());
	}
	SIValue converted = SI_LongVal(l);
	return AR_EXP_NewConstOperandNode(converted);
}

static AR_ExpNode *_AR_EXP_FromFloatExpression(const cypher_astnode_t *expr) {
	const char *value_str = cypher_ast_float_get_valuestr(expr);
	char *endptr = NULL;
	double d = strtod(value_str, &endptr);
	if(endptr[0] != 0) {
		// Failed to convert integer value; set compile-time error to be raised later.
		QueryCtx_SetError("Invalid numeric value '%s'", value_str);
		return AR_EXP_NewConstOperandNode(SI_NullVal());
	}
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

static AR_ExpNode *_AR_ExpFromNamedPath(const cypher_astnode_t *path) {
	uint path_len = cypher_ast_pattern_path_nelements(path);
	/* The method TO_PATH accepts as its first parameter the ast node which represents the path.
	 * The other parameters are the graph entities (node, edge, path) which the path builder implemented
	 * in TO_PATH requires in order to build a complete path. The order of the evaluated graph entities
	 * is the same order in which they apeare in the AST.*/
	AR_ExpNode *op = AR_EXP_NewOpNode("topath", 1 + path_len);
	// Set path AST as first paramerter.
	op->op.children[0] = AR_EXP_NewConstOperandNode(SI_PtrVal((void *)path));
	for(uint i = 0; i < path_len; i ++)
		// Set graph entities as parameters, ordered according to the path AST.
		op->op.children[i + 1] = _AR_EXP_FromExpression(cypher_ast_pattern_path_get_element(path, i));
	return op;
}

static AR_ExpNode *_AR_ExpNodeFromGraphEntity(const cypher_astnode_t *entity) {
	AST *ast = QueryCtx_GetAST();
	const char *alias = AST_GetEntityName(ast, entity);
	return AR_EXP_NewVariableOperandNode(alias, NULL);
}

static AR_ExpNode *_AR_ExpNodeFromParameter(const cypher_astnode_t *param) {
	const char *identifier = cypher_ast_parameter_get_name(param);
	return AR_EXP_NewParameterOperandNode(identifier);
}

static AR_ExpNode *_AR_EXP_FromExpression(const cypher_astnode_t *expr) {

	const cypher_astnode_type_t type = cypher_astnode_type(expr);

	/* Function invocations */
	if(type == CYPHER_AST_APPLY_OPERATOR) {
		return _AR_EXP_FromApplyExpression(expr);
		/* Function invocations with STAR projections */
	} else if(type == CYPHER_AST_APPLY_ALL_OPERATOR) {
		return _AR_EXP_FromApplyAllExpression(expr);
		/* Variables (full nodes and edges, UNWIND artifacts */
	} else if(type == CYPHER_AST_IDENTIFIER) {
		// Check if the identifier is a named path identifier.
		AST *ast = QueryCtx_GetAST();
		AnnotationCtx *named_paths_ctx = AST_AnnotationCtxCollection_GetNamedPathsCtx(
											 ast->anot_ctx_collection);
		const cypher_astnode_t *named_path_annotation = cypher_astnode_get_annotation(named_paths_ctx,
																					  expr);
		// If the identifier is a named path identifier, evaluate the path expression accordingly.
		if(named_path_annotation) return _AR_EXP_FromExpression(named_path_annotation);
		// Else, evalute the identifier.
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
	} else if(type == CYPHER_AST_NAMED_PATH) {
		return _AR_ExpFromNamedPath(expr);
	} else if(type == CYPHER_AST_NODE_PATTERN || type == CYPHER_AST_REL_PATTERN) {
		return _AR_ExpNodeFromGraphEntity(expr);
	} else if(type == CYPHER_AST_PARAMETER) {
		return _AR_ExpNodeFromParameter(expr);
	} else {
		/*
		   Unhandled types:
		   CYPHER_AST_LABELS_OPERATOR
		   CYPHER_AST_LIST_COMPREHENSION
		   CYPHER_AST_MAP
		   CYPHER_AST_MAP_PROJECTION
		   CYPHER_AST_PATTERN_COMPREHENSION
		   CYPHER_AST_REDUCE
		*/
		const char *type_str = cypher_astnode_typestr(type);
		QueryCtx_SetError("RedisGraph does not currently support the type '%s'", type_str);
		return AR_EXP_NewConstOperandNode(SI_NullVal());
	}

	assert(false);
	return NULL;
}

AR_ExpNode *AR_EXP_FromExpression(const cypher_astnode_t *expr) {
	AR_ExpNode *root = _AR_EXP_FromExpression(expr);
	AR_EXP_ReduceToScalar(root, false, NULL);

	/* Make sure expression doesn't contains nested aggregation functions
	 * count(max(n.v)) */
	if(_AR_EXP_ContainsNestedAgg(root)) {
		// Set error (compile-time), this error will be raised later on.
		QueryCtx_SetError("Can't use aggregate functions inside of aggregate functions.");
	}

	return root;
}

