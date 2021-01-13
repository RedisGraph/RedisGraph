/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "arithmetic_expression_construct.h"
#include "../RG.h"
#include "funcs.h"
#include "../errors.h"
#include "../query_ctx.h"
#include "../util/rmalloc.h"
#include "../ast/ast_build_filter_tree.h"

// Forward declaration
static AR_ExpNode *_AR_EXP_FromASTNode(const cypher_astnode_t *expr);

static bool __AR_EXP_ContainsNestedAgg(const AR_ExpNode *root, bool in_agg) {
	// Is this an aggregation node?
	bool agg_node = (root->type == AR_EXP_OP && root->op.f->aggregate == true);
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

#define OP_COUNT 24
// The OpName array is strictly parallel with the AST_Operator enum.
static const char *OpName[OP_COUNT] = {
	"UNKNOWN", "NULL", "OR", "XOR", "AND", "NOT", "EQ", "NEQ", "LT", "GT", "LE",  "GE",
	"ADD", "SUB", "MUL", "DIV", "MOD", "POW", "CONTAINS", "STARTS WITH",
	"ENDS WITH", "IN", "IS NULL", "IS NOT NULL"
};

static inline const char *_ASTOpToString(AST_Operator op) {
	ASSERT(op < OP_COUNT);
	return OpName[op];
}

static AR_ExpNode *AR_EXP_NewOpNodeFromAST(AST_Operator op, uint child_count) {
	const char *func_name = _ASTOpToString(op);
	return AR_EXP_NewOpNode(func_name, child_count);
}

static AR_ExpNode *_AR_EXP_FromApplyExpression(const cypher_astnode_t *expr) {
	AR_ExpNode *op;
	bool distinct = cypher_ast_apply_operator_get_distinct(expr);
	unsigned int arg_count = cypher_ast_apply_operator_narguments(expr);
	const cypher_astnode_t *func_node = cypher_ast_apply_operator_get_func_name(expr);
	const char *func_name = cypher_ast_function_name_get_value(func_node);
	bool aggregate = AR_FuncIsAggregate(func_name);
	op = AR_EXP_NewOpNode(func_name, arg_count);

	for(unsigned int i = 0; i < arg_count; i ++) {
		const cypher_astnode_t *arg = cypher_ast_apply_operator_get_argument(expr, i);
		// Recursively convert arguments
		op->op.children[i] = _AR_EXP_FromASTNode(arg);
	}

	if(aggregate) {
		AggregateCtx *ctx = rm_malloc(sizeof(AggregateCtx));
		ctx->hashSet = NULL;
		ctx->private_ctx = NULL;
		ctx->result = SI_NullVal();
		if(distinct) ctx->hashSet = Set_New();
		// Add the context to the function descriptor as the function's private data.
		op->op.f = AR_SetPrivateData(op->op.f, ctx);
	}

	return op;
}

static AR_ExpNode *_AR_EXP_FromApplyAllExpression(const cypher_astnode_t *expr) {
	// ApplyAll operators use accessors similar to normal Apply operators with the exception
	// that they have no argument accessors - by definition, they have one argument (all/STAR).
	const cypher_astnode_t *func_node = cypher_ast_apply_all_operator_get_func_name(expr);
	const char *func_name = cypher_ast_function_name_get_value(func_node);
	bool aggregate = AR_FuncIsAggregate(func_name);
	AR_ExpNode *op = AR_EXP_NewOpNode(func_name, 1);

	// Introduce a fake child constant so that the function always operates on something.
	op->op.children[0] = AR_EXP_NewConstOperandNode(SI_BoolVal(1));

	if(aggregate) {
		AggregateCtx *ctx = rm_malloc(sizeof(AggregateCtx));
		ctx->hashSet = NULL;
		ctx->private_ctx = NULL;
		ctx->result = SI_NullVal();
		// Add the context to the function descriptor as the function's private data.
		op->op.f = AR_SetPrivateData(op->op.f, ctx);
	}

	return op;
}

static AR_ExpNode *_AR_EXP_FromIdentifierExpression(const cypher_astnode_t *expr) {
	// Identifier referencing another entity
	const char *alias = cypher_ast_identifier_get_name(expr);
	return AR_EXP_NewVariableOperandNode(alias);
}

static AR_ExpNode *_AR_EXP_FromIdentifier(const cypher_astnode_t *expr) {
	AST *ast = QueryCtx_GetAST();
	if(ast == NULL) {
		/* Attempted to access the AST before it has been constructed.
		 * This can occur in scenarios like parameter evaluation:
		 * CYPHER param=[a] MATCH (a) RETURN a */
		ErrorCtx_SetError("Attempted to access variable before it has been defined");
		return AR_EXP_NewConstOperandNode(SI_NullVal());
	}

	// check if the identifier is a named path identifier
	AnnotationCtx *named_paths_ctx =
		AST_AnnotationCtxCollection_GetNamedPathsCtx(ast->anot_ctx_collection);

	const cypher_astnode_t *named_path_annotation =
		cypher_astnode_get_annotation(named_paths_ctx, expr);

	// if the identifier is a named path identifier,
	// evaluate the path expression accordingly
	if(named_path_annotation) return _AR_EXP_FromASTNode(named_path_annotation);
	// else, evalute the identifier
	return _AR_EXP_FromIdentifierExpression(expr);
}

static AR_ExpNode *_AR_EXP_FromPropertyExpression(const cypher_astnode_t *expr) {
	// the property expression is constructed of two parts:
	// 1. an expression evaluating to a graph entity, in the future a map type
	// 2. a property name string.
	// examples: a.v, arr[0].v

	//--------------------------------------------------------------------------
	// Extract entity and property name expressions.
	//--------------------------------------------------------------------------

	const cypher_astnode_t *prop_expr = cypher_ast_property_operator_get_expression(expr);
	const cypher_astnode_t *prop_name_node = cypher_ast_property_operator_get_prop_name(expr);
	const char *prop_name = cypher_ast_prop_name_get_value(prop_name_node);

	AR_ExpNode *entity = _AR_EXP_FromASTNode(prop_expr);
	AR_ExpNode *root = AR_EXP_NewAttributeAccessNode(entity, prop_name);

	return root;
}

static AR_ExpNode *_AR_EXP_FromIntegerExpression(const cypher_astnode_t *expr) {
	const char *value_str = cypher_ast_integer_get_valuestr(expr);
	char *endptr = NULL;
	int64_t l = strtol(value_str, &endptr, 0);
	if(endptr[0] != 0) {
		// Failed to convert integer value; set compile-time error to be raised later.
		ErrorCtx_SetError("Invalid numeric value '%s'", value_str);
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
		ErrorCtx_SetError("Invalid numeric value '%s'", value_str);
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
		op->op.children[1] = _AR_EXP_FromASTNode(arg);
	} else if(operator == CYPHER_OP_UNARY_PLUS) {
		/* This expression is something like +3 or +a.val.
		 * I think the + can always be safely ignored. */
		op = _AR_EXP_FromASTNode(arg);
	} else if(operator == CYPHER_OP_NOT) {
		op = AR_EXP_NewOpNodeFromAST(OP_NOT, 1);
		op->op.children[0] = _AR_EXP_FromASTNode(arg);
	} else if(operator == CYPHER_OP_IS_NULL) {
		op = AR_EXP_NewOpNodeFromAST(OP_IS_NULL, 1);
		op->op.children[0] = _AR_EXP_FromASTNode(arg);
	} else if(operator == CYPHER_OP_IS_NOT_NULL) {
		op = AR_EXP_NewOpNodeFromAST(OP_IS_NOT_NULL, 1);
		op->op.children[0] = _AR_EXP_FromASTNode(arg);
	} else {
		// No supported operator found.
		ASSERT(false);
	}
	return op;
}

static AR_ExpNode *_AR_EXP_FromBinaryOpExpression(const cypher_astnode_t *expr) {
	const cypher_operator_t *operator = cypher_ast_binary_operator_get_operator(expr);
	AST_Operator operator_enum = AST_ConvertOperatorNode(operator);
	// Arguments are of type CYPHER_AST_EXPRESSION
	AR_ExpNode *op = AR_EXP_NewOpNodeFromAST(operator_enum, 2);
	const cypher_astnode_t *lhs_node = cypher_ast_binary_operator_get_argument1(expr);
	op->op.children[0] = _AR_EXP_FromASTNode(lhs_node);
	const cypher_astnode_t *rhs_node = cypher_ast_binary_operator_get_argument2(expr);
	op->op.children[1] = _AR_EXP_FromASTNode(rhs_node);
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
			inner_op->op.children[0] = _AR_EXP_FromASTNode(lhs_node);
			inner_op->op.children[1] = _AR_EXP_FromASTNode(rhs_node);
			op->op.children[i] = inner_op;
		}
	} else {
		const cypher_operator_t *operator = cypher_ast_comparison_get_operator(expr, 0);
		AST_Operator operator_enum = AST_ConvertOperatorNode(operator);
		op = AR_EXP_NewOpNodeFromAST(operator_enum, 2);
		const cypher_astnode_t *lhs_node = cypher_ast_comparison_get_argument(expr, 0);
		const cypher_astnode_t *rhs_node = cypher_ast_comparison_get_argument(expr, 1);
		op->op.children[0] = _AR_EXP_FromASTNode(lhs_node);
		op->op.children[1] = _AR_EXP_FromASTNode(rhs_node);
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
		op->op.children[offset++] = _AR_EXP_FromASTNode(expression);
	}

	// Alternatives
	for(uint i = 0; i < alternatives; i++) {
		const cypher_astnode_t *predicate = cypher_ast_case_get_predicate(expr, i);
		op->op.children[offset++] = _AR_EXP_FromASTNode(predicate);
		const cypher_astnode_t *value = cypher_ast_case_get_value(expr, i);
		op->op.children[offset++] = _AR_EXP_FromASTNode(value);
	}

	// Default value.
	const cypher_astnode_t *deflt = cypher_ast_case_get_default(expr);
	if(deflt == NULL) {
		// Default not specified, use NULL.
		op->op.children[offset] = AR_EXP_NewConstOperandNode(SI_NullVal());
	} else {
		op->op.children[offset] = _AR_EXP_FromASTNode(deflt);
	}

	return op;
}

static AR_ExpNode *_AR_ExpFromCollectionExpression(const cypher_astnode_t *expr) {
	uint expCount = cypher_ast_collection_length(expr);
	AR_ExpNode *op = AR_EXP_NewOpNode("tolist", expCount);
	for(uint i = 0; i < expCount; i ++) {
		const cypher_astnode_t *exp_node = cypher_ast_collection_get(expr, i);
		op->op.children[i] = AR_EXP_FromASTNode(exp_node);
	}
	return op;
}

static AR_ExpNode *_AR_ExpFromMapExpression(const cypher_astnode_t *expr) {
	/* create a new map construction expression
	 * determine number of elements in map
	 * double argument count to accommodate both key and value */
	uint element_count = cypher_ast_map_nentries(expr);
	AR_ExpNode *op = AR_EXP_NewOpNode("tomap", element_count * 2);

	// process each key value pair
	for(uint i = 0; i < element_count; i++) {
		const cypher_astnode_t *key_node = cypher_ast_map_get_key(expr, i);
		const char *key = cypher_ast_prop_name_get_value(key_node);
		const cypher_astnode_t *val = cypher_ast_map_get_value(expr, i);
		// this is a bit of an overkill, as key is supposed to be a const string
		op->op.children[i*2] = AR_EXP_NewConstOperandNode(SI_ConstStringVal((char*)key));
		op->op.children[i*2+1] = AR_EXP_FromASTNode(val);
	}

	return op;
}

static AR_ExpNode *_AR_ExpFromMapProjection(const cypher_astnode_t *expr) {
	// MATCH (n) RETURN n { .name, .age, scores: collect(m.score) }

	cypher_astnode_type_t t;
	const cypher_astnode_t *identifier = cypher_ast_map_projection_get_expression(expr);
	AR_ExpNode *entity = AR_EXP_FromASTNode(identifier);

	const cypher_astnode_t *selector = NULL;
	unsigned int n_selectors = cypher_ast_map_projection_nselectors(expr);

	AR_ExpNode *op = AR_EXP_NewOpNode("tomap", n_selectors*2);
	AR_ExpNode **children = op->op.children;

	for(uint i = 0; i < n_selectors; i++) {
		selector = cypher_ast_map_projection_get_selector(expr, i);

		const char *prop_name = NULL;
		const cypher_astnode_t *prop = NULL;
		t = cypher_astnode_type(selector);

		if(t == CYPHER_AST_MAP_PROJECTION_PROPERTY) {
			// { .name }
			prop = cypher_ast_map_projection_property_get_prop_name(selector);
			prop_name = cypher_ast_prop_name_get_value(prop);

			children[i*2] =
				AR_EXP_NewConstOperandNode(SI_ConstStringVal((char*)prop_name));

			children[i*2+1] = AR_EXP_NewAttributeAccessNode(entity, prop_name);
		} else if(t == CYPHER_AST_MAP_PROJECTION_LITERAL) {
			// { v: n.v }
			prop = cypher_ast_map_projection_literal_get_prop_name(selector);
			prop_name = cypher_ast_prop_name_get_value(prop);
			const cypher_astnode_t *literal_exp =
				cypher_ast_map_projection_literal_get_expression(selector);

			children[i*2] =
				AR_EXP_NewConstOperandNode(SI_ConstStringVal((char*)prop_name));

			children[i*2+1] = AR_EXP_FromASTNode(literal_exp);
		} else {
			ASSERT("Unexpected AST node type" && false);
		}
	}

	return op;
}

static AR_ExpNode *_AR_ExpFromSubscriptExpression(const cypher_astnode_t *expr) {
	AR_ExpNode *op = AR_EXP_NewOpNode("subscript", 2);
	const cypher_astnode_t *exp_node = cypher_ast_subscript_operator_get_expression(expr);
	op->op.children[0] = AR_EXP_FromASTNode(exp_node);
	const cypher_astnode_t *subscript_node = cypher_ast_subscript_operator_get_subscript(expr);
	op->op.children[1] = AR_EXP_FromASTNode(subscript_node);
	return op;
}

static AR_ExpNode *_AR_ExpFromSliceExpression(const cypher_astnode_t *expr) {
	AR_ExpNode *op = AR_EXP_NewOpNode("slice", 3);
	const cypher_astnode_t *exp_node = cypher_ast_slice_operator_get_expression(expr);
	const cypher_astnode_t *start_node = cypher_ast_slice_operator_get_start(expr);
	const cypher_astnode_t *end_node = cypher_ast_slice_operator_get_end(expr);

	op->op.children[0] = AR_EXP_FromASTNode(exp_node);

	if(start_node) op->op.children[1] = AR_EXP_FromASTNode(start_node);
	else op->op.children[1] = AR_EXP_NewConstOperandNode(SI_LongVal(0));

	if(end_node) op->op.children[2] = AR_EXP_FromASTNode(end_node);
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
		op->op.children[i + 1] = _AR_EXP_FromASTNode(cypher_ast_pattern_path_get_element(path, i));
	return op;
}

static AR_ExpNode *_AR_ExpNodeFromGraphEntity(const cypher_astnode_t *entity) {
	AST *ast = QueryCtx_GetAST();
	const char *alias = AST_GetEntityName(ast, entity);
	return AR_EXP_NewVariableOperandNode(alias);
}

static AR_ExpNode *_AR_ExpNodeFromParameter(const cypher_astnode_t *param) {
	const char *identifier = cypher_ast_parameter_get_name(param);
	return AR_EXP_NewParameterOperandNode(identifier);
}

static AR_ExpNode *_AR_ExpNodeFromComprehensionFunction(const cypher_astnode_t *comp_exp,
														cypher_astnode_type_t type) {
	// Set the appropriate function name according to the node type.
	const char *func_name;
	if(type == CYPHER_AST_ANY) func_name = "ANY";
	else if(type == CYPHER_AST_ALL) func_name = "ALL";
	else func_name = "LIST_COMPREHENSION";

	/* Using the sample query:
	 * WITH [1,2,3] AS arr RETURN [val IN arr WHERE val % 2 = 1 | val * 2] AS comp
	 */

	/* The comprehension's local variable, WHERE expression, and eval routine
	 * do not change for each invocation, so are bundled together in the function's context. */
	ListComprehensionCtx *ctx = rm_malloc(sizeof(ListComprehensionCtx));
	ctx->ft = NULL;
	ctx->eval_exp = NULL;
	ctx->local_record = NULL;
	ctx->variable_idx = INVALID_INDEX;
	/* Retrieve the variable name introduced in this context to iterate over list elements.
	 * In the above query, this is 'val'. */
	const cypher_astnode_t *variable_node = cypher_ast_list_comprehension_get_identifier(comp_exp);
	ASSERT(cypher_astnode_type(variable_node) == CYPHER_AST_IDENTIFIER);
	// Retrieve the variable string for the local variable.
	ctx->variable_str = cypher_ast_identifier_get_name(variable_node);

	/* The predicate node is the set of WHERE conditions in the comprehension, if any. */
	const cypher_astnode_t *predicate_node = cypher_ast_list_comprehension_get_predicate(comp_exp);
	// Build a FilterTree to represent this predicate.
	if(predicate_node) {
		AST_ConvertFilters(&ctx->ft, predicate_node);
	} else if(type != CYPHER_AST_LIST_COMPREHENSION) {
		// Functions like any() and all() must have a predicate node.
		ErrorCtx_SetError("'%s' function requires a WHERE predicate", func_name);
		rm_free(ctx);
		return AR_EXP_NewConstOperandNode(SI_NullVal());
	}

	/* Construct the operator node that will generate updated values, if one is provided.
	 * In the above query, this will be an operation node representing "val * 2".
	 * This will always be NULL for comprehensions like any() and all(). */
	const cypher_astnode_t *eval_node = cypher_ast_list_comprehension_get_eval(comp_exp);
	if(eval_node) ctx->eval_exp = _AR_EXP_FromASTNode(eval_node);

	// Build an operation node to represent the list comprehension.
	AR_ExpNode *op = AR_EXP_NewOpNode(func_name, 2);

	// Add the context to the function descriptor as the function's private data.
	op->op.f = AR_SetPrivateData(op->op.f, ctx);

	/* 'arr' is the list expression.
	 * Note that this value could resolve to an alias, a literal array, a function call, and so on. */
	const cypher_astnode_t *list_node = cypher_ast_list_comprehension_get_expression(comp_exp);
	AR_ExpNode *list = _AR_EXP_FromASTNode(list_node);

	// The list expression is the function's first child.
	op->op.children[0] = list;

	// The function's second child will be a pointer to the Record being evaluated.
	op->op.children[1] = AR_EXP_NewRecordNode();

	return op;
}

static AR_ExpNode *_AR_EXP_FromASTNode(const cypher_astnode_t *expr) {

	const cypher_astnode_type_t t = cypher_astnode_type(expr);

	// function invocations
	if(t == CYPHER_AST_APPLY_OPERATOR) {
		return _AR_EXP_FromApplyExpression(expr);
		// function invocations with STAR projections
	} else if(t == CYPHER_AST_APPLY_ALL_OPERATOR) {
		return _AR_EXP_FromApplyAllExpression(expr);
		// variables (full nodes and edges, UNWIND artifacts
	} else if(t == CYPHER_AST_IDENTIFIER) {
		return _AR_EXP_FromIdentifier(expr);
		// entity-property pair
	} else if(t == CYPHER_AST_PROPERTY_OPERATOR) {
		return _AR_EXP_FromPropertyExpression(expr);
		// sIValue constant types
	} else if(t == CYPHER_AST_INTEGER) {
		return _AR_EXP_FromIntegerExpression(expr);
	} else if(t == CYPHER_AST_FLOAT) {
		return _AR_EXP_FromFloatExpression(expr);
	} else if(t == CYPHER_AST_STRING) {
		return _AR_EXP_FromStringExpression(expr);
	} else if(t == CYPHER_AST_TRUE) {
		return _AR_EXP_FromTrueExpression();
	} else if(t == CYPHER_AST_FALSE) {
		return _AR_EXP_FromFalseExpression();
	} else if(t == CYPHER_AST_NULL) {
		return _AR_EXP_FromNullExpression();
		// handling for unary operators (-5, +a.val)
	} else if(t == CYPHER_AST_UNARY_OPERATOR) {
		return _AR_EXP_FromUnaryOpExpression(expr);
	} else if(t == CYPHER_AST_BINARY_OPERATOR) {
		return _AR_EXP_FromBinaryOpExpression(expr);
	} else if(t == CYPHER_AST_COMPARISON) {
		return _AR_EXP_FromComparisonExpression(expr);
	} else if(t == CYPHER_AST_CASE) {
		return _AR_EXP_FromCaseExpression(expr);
	} else if(t == CYPHER_AST_COLLECTION) {
		return _AR_ExpFromCollectionExpression(expr);
	} else if(t == CYPHER_AST_SUBSCRIPT_OPERATOR) {
		return _AR_ExpFromSubscriptExpression(expr);
	} else if(t == CYPHER_AST_SLICE_OPERATOR) {
		return _AR_ExpFromSliceExpression(expr);
	} else if(t == CYPHER_AST_NAMED_PATH) {
		return _AR_ExpFromNamedPath(expr);
	} else if(t == CYPHER_AST_NODE_PATTERN || t == CYPHER_AST_REL_PATTERN) {
		return _AR_ExpNodeFromGraphEntity(expr);
	} else if(t == CYPHER_AST_PARAMETER) {
		return _AR_ExpNodeFromParameter(expr);
	} else if(t == CYPHER_AST_LIST_COMPREHENSION || t == CYPHER_AST_ANY ||
			  t == CYPHER_AST_ALL) {
		return _AR_ExpNodeFromComprehensionFunction(expr, t);
	} else if(t == CYPHER_AST_MAP) {
		return _AR_ExpFromMapExpression(expr);
	} else if(t == CYPHER_AST_MAP_PROJECTION) {
		return _AR_ExpFromMapProjection(expr);
	} else {
		/*
		   Unhandled types:
		   CYPHER_AST_LABELS_OPERATOR
		   CYPHER_AST_PATTERN_COMPREHENSION
		   CYPHER_AST_REDUCE
		*/
		Error_UnsupportedASTNodeType(expr);
		return AR_EXP_NewConstOperandNode(SI_NullVal());
	}

	ASSERT(false);
	return NULL;
}

AR_ExpNode *AR_EXP_FromASTNode(const cypher_astnode_t *expr) {
	AR_ExpNode *root = _AR_EXP_FromASTNode(expr);
	AR_EXP_ReduceToScalar(root, false, NULL);

	/* Make sure expression doesn't contains nested aggregation functions
	 * count(max(n.v)) */
	if(_AR_EXP_ContainsNestedAgg(root)) {
		// Set error (compile-time), this error will be raised later on.
		ErrorCtx_SetError("Can't use aggregate functions inside of aggregate functions.");
	}

	return root;
}

