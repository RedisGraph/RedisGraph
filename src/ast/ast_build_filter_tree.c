#include "ast_build_filter_tree.h"
#include "ast_build_ar_exp.h"
#include "ast_shared.h"
#include "../execution_plan/record_map.h"
#include "../util/arr.h"

// Forward declaration
FT_FilterNode *_FilterNode_FromAST(RecordMap *record_map, const cypher_astnode_t *expr);

FT_FilterNode *_CreatePredicateFilterNode(RecordMap *record_map, AST_Operator op,
										  const cypher_astnode_t *lhs, const cypher_astnode_t *rhs) {
	return FilterTree_CreatePredicateFilter(op, AR_EXP_FromExpression(record_map, lhs),
											AR_EXP_FromExpression(record_map, rhs));
}

void _FT_Append(FT_FilterNode **root_ptr, FT_FilterNode *child) {
	assert(child);

	FT_FilterNode *root = *root_ptr;
	// If the tree is uninitialized, its root is the child
	if(root == NULL) {
		*root_ptr = child;
		return;
	}

	if(root->t == FT_N_COND) {
		if(root->cond.left == NULL) {
			AppendLeftChild(root, child);
			return;
		}
		if(root->cond.right == NULL) {
			AppendRightChild(root, child);
			return;
		}
	}

	FT_FilterNode *new_root = FilterTree_CreateConditionFilter(OP_AND);
	AppendLeftChild(new_root, root);
	AppendRightChild(new_root, child);
	*root_ptr = new_root;
}

FT_FilterNode *_CreateFilterSubtree(RecordMap *record_map, AST_Operator op,
									const cypher_astnode_t *lhs, const cypher_astnode_t *rhs) {
	FT_FilterNode *filter = NULL;
	switch(op) {
	case OP_OR:
	case OP_AND:
		filter = FilterTree_CreateConditionFilter(op);
		AppendLeftChild(filter, _FilterNode_FromAST(record_map, lhs));
		AppendRightChild(filter, _FilterNode_FromAST(record_map, rhs));
		return filter;
	case OP_NOT:
		filter = FilterTree_CreateConditionFilter(op);
		AppendLeftChild(filter, _FilterNode_FromAST(record_map, lhs));
		AppendRightChild(filter, NULL);
		return filter;
	case OP_EQUAL:
	case OP_NEQUAL:
	case OP_LT:
	case OP_LE:
	case OP_GT:
	case OP_GE:
		return _CreatePredicateFilterNode(record_map, op, lhs, rhs);
	default:
		assert("attempted to convert unhandled type into filter" && false);
	}
}

// AND, OR, XOR (others?)
/* WHERE (condition) AND (condition),
 * WHERE a.val = b.val */
static FT_FilterNode *_convertBinaryOperator(RecordMap *record_map,
											 const cypher_astnode_t *op_node) {
	const cypher_operator_t *operator = cypher_ast_binary_operator_get_operator(op_node);
	AST_Operator op = AST_ConvertOperatorNode(operator);
	const cypher_astnode_t *lhs;
	const cypher_astnode_t *rhs;

	switch(op) {
	case OP_OR:
	case OP_AND:
	case OP_NOT:
	case OP_EQUAL:
	case OP_NEQUAL:
	case OP_LT:
	case OP_LE:
	case OP_GT:
	case OP_GE:
		// Arguments are of type CYPHER_AST_EXPRESSION
		lhs = cypher_ast_binary_operator_get_argument1(op_node);
		rhs = cypher_ast_binary_operator_get_argument2(op_node);
		return _CreateFilterSubtree(record_map, op, lhs, rhs);
	default:
		return FilterTree_CreateExpressionFilter(AR_EXP_FromExpression(record_map, op_node));
	}
}

static FT_FilterNode *_convertUnaryOperator(RecordMap *record_map,
											const cypher_astnode_t *op_node) {
	const cypher_operator_t *operator = cypher_ast_unary_operator_get_operator(op_node);
	// Argument is of type CYPHER_AST_EXPRESSION
	const cypher_astnode_t *arg = cypher_ast_unary_operator_get_argument(op_node);
	AST_Operator op = AST_ConvertOperatorNode(operator);

	return _CreateFilterSubtree(record_map, op, arg, NULL);
}

static FT_FilterNode *_convertApplyOperator(RecordMap *record_map,
											const cypher_astnode_t *op_node) {
	return FilterTree_CreateExpressionFilter(AR_EXP_FromExpression(record_map, op_node));
}

static FT_FilterNode *_convertTrueOperator() {
	AR_ExpNode *exp = AR_EXP_NewConstOperandNode(SI_BoolVal(true));
	return FilterTree_CreateExpressionFilter(exp);
}

static FT_FilterNode *_convertFalseOperator() {
	AR_ExpNode *exp = AR_EXP_NewConstOperandNode(SI_BoolVal(false));
	return FilterTree_CreateExpressionFilter(exp);
}

static FT_FilterNode *_convertIntegerOperator(RecordMap *record_map, const cypher_astnode_t *expr) {
	AR_ExpNode *exp = AR_EXP_FromExpression(record_map, expr);
	return FilterTree_CreateExpressionFilter(exp);
}

/* A comparison node contains two arrays - one of operators, and one of expressions.
 * Most comparisons will only have one operator and two expressions, but Cypher
 * allows more complex formulations like "x < y <= z".
 * A comparison takes a form such as "WHERE a.val < y.val". */
static FT_FilterNode *_convertComparison(RecordMap *record_map,
										 const cypher_astnode_t *comparison_node) {
	// "x < y <= z"
	uint nelems = cypher_ast_comparison_get_length(comparison_node);
	FT_FilterNode **filters = array_new(FT_FilterNode *, nelems);

	// Create and accumulate simple predicates x < y.
	for(int i = 0; i < nelems; i++) {
		const cypher_operator_t *operator = cypher_ast_comparison_get_operator(comparison_node, i);
		const cypher_astnode_t *lhs = cypher_ast_comparison_get_argument(comparison_node, i);
		const cypher_astnode_t *rhs = cypher_ast_comparison_get_argument(comparison_node, i + 1);

		AST_Operator op = AST_ConvertOperatorNode(operator);
		FT_FilterNode *filter = _CreatePredicateFilterNode(record_map, op, lhs, rhs);
		filters = array_append(filters, filter);
	}

	// Reduce by anding.
	while(array_len(filters) > 1) {
		FT_FilterNode *a = array_pop(filters);
		FT_FilterNode *b = array_pop(filters);
		FT_FilterNode *intersec = FilterTree_CreateConditionFilter(OP_AND);

		AppendLeftChild(intersec, a);
		AppendRightChild(intersec, b);

		filters = array_append(filters, intersec);
	}

	FT_FilterNode *root = array_pop(filters);
	array_free(filters);
	return root;
}

static FT_FilterNode *_convertInlinedProperties(RecordMap *record_map, const AST *ast,
												const cypher_astnode_t *entity, GraphEntityType type) {
	const cypher_astnode_t *props = NULL;

	if(type == GETYPE_NODE) {
		props = cypher_ast_node_pattern_get_properties(entity);
	} else { // relation
		props = cypher_ast_rel_pattern_get_properties(entity);
	}

	if(!props) return NULL;

	// Retrieve the AST ID of the entity.
	uint ast_id = AST_GetEntityIDFromReference(ast, entity);

	// Add the AST ID to the record map.
	uint record_id = RecordMap_FindOrAddID(record_map, ast_id);

	FT_FilterNode *root = NULL;
	uint nelems = cypher_ast_map_nentries(props);
	for(uint i = 0; i < nelems; i ++) {
		// key is of type CYPHER_AST_PROP_NAME
		const char *prop = cypher_ast_prop_name_get_value(cypher_ast_map_get_key(props, i));
		AR_ExpNode *lhs = AR_EXP_NewVariableOperandNode(record_map, NULL, prop);
		lhs->operand.variadic.entity_alias_idx = record_id;
		// val is of type CYPHER_AST_EXPRESSION
		const cypher_astnode_t *val = cypher_ast_map_get_value(props, i);
		AR_ExpNode *rhs = AR_EXP_FromExpression(record_map, val);
		/* TODO In a query like:
		 * "MATCH (r:person {name:"Roi"}) RETURN r"
		 * (note the repeated double quotes) - this creates a variable rather than a scalar.
		 * Can we use this to handle escape characters or something? How does it work? */
		// Inlined properties can only be scalars right now
		assert(rhs->operand.type == AR_EXP_CONSTANT &&
			   "non-scalar inlined property are not currently supported.");
		FT_FilterNode *t = FilterTree_CreatePredicateFilter(OP_EQUAL, lhs, rhs);
		_FT_Append(&root, t);
	}
	return root;
}

FT_FilterNode *_FilterNode_FromAST(RecordMap *record_map, const cypher_astnode_t *expr) {
	assert(expr);
	cypher_astnode_type_t type = cypher_astnode_type(expr);

	if(type == CYPHER_AST_COMPARISON) {
		return _convertComparison(record_map, expr);
	} else if(type == CYPHER_AST_BINARY_OPERATOR) {
		return _convertBinaryOperator(record_map, expr);
	} else if(type == CYPHER_AST_UNARY_OPERATOR) {
		return _convertUnaryOperator(record_map, expr);
	} else if(type == CYPHER_AST_APPLY_OPERATOR) {
		return _convertApplyOperator(record_map, expr);
	} else if(type == CYPHER_AST_TRUE) {
		return _convertTrueOperator();
	} else if(type == CYPHER_AST_FALSE) {
		return _convertFalseOperator();
	} else if(type == CYPHER_AST_INTEGER) {
		return _convertIntegerOperator(record_map, expr);
	} else {
		assert(false);
		return NULL;
	}
}

void _AST_ConvertFilters(RecordMap *record_map, const AST *ast,
						 FT_FilterNode **root, const cypher_astnode_t *entity) {
	if(!entity) return;

	cypher_astnode_type_t type = cypher_astnode_type(entity);

	FT_FilterNode *node = NULL;
	// If the current entity is a node or edge pattern, capture its properties map (if any)
	if(type == CYPHER_AST_NODE_PATTERN) {
		node = _convertInlinedProperties(record_map, ast, entity, GETYPE_NODE);
	} else if(type == CYPHER_AST_REL_PATTERN) {
		node = _convertInlinedProperties(record_map, ast, entity, GETYPE_EDGE);
	} else if(type == CYPHER_AST_COMPARISON) {
		node = _convertComparison(record_map, entity);
	} else if(type == CYPHER_AST_BINARY_OPERATOR) {
		node = _convertBinaryOperator(record_map, entity);
	} else if(type == CYPHER_AST_UNARY_OPERATOR) {
		node = _convertUnaryOperator(record_map, entity);
	} else if(type == CYPHER_AST_APPLY_OPERATOR) {
		node = _convertApplyOperator(record_map, entity);
	} else if(type == CYPHER_AST_TRUE) {
		node = _convertTrueOperator();
	} else if(type == CYPHER_AST_FALSE) {
		node = _convertFalseOperator();
	} else if(type == CYPHER_AST_INTEGER) {
		node = _convertIntegerOperator(record_map, entity);
	} else {
		uint child_count = cypher_astnode_nchildren(entity);
		for(uint i = 0; i < child_count; i++) {
			const cypher_astnode_t *child = cypher_astnode_get_child(entity, i);
			// Recursively continue searching
			_AST_ConvertFilters(record_map, ast, root, child);
		}
	}
	if(node) _FT_Append(root, node);
}

FT_FilterNode *AST_BuildFilterTree(AST *ast, RecordMap *record_map) {
	FT_FilterNode *filter_tree = NULL;
	const cypher_astnode_t **match_clauses = AST_GetClauses(ast, CYPHER_AST_MATCH);
	if(match_clauses) {
		uint match_count = array_len(match_clauses);
		for(uint i = 0; i < match_count; i ++) {
			_AST_ConvertFilters(record_map, ast, &filter_tree, match_clauses[i]);
		}
		array_free(match_clauses);
	}

	const cypher_astnode_t **merge_clauses = AST_GetClauses(ast, CYPHER_AST_MERGE);
	if(merge_clauses) {
		uint merge_count = array_len(merge_clauses);
		for(uint i = 0; i < merge_count; i ++) {
			_AST_ConvertFilters(record_map, ast, &filter_tree, merge_clauses[i]);
		}
		array_free(merge_clauses);
	}

	// Apply De Morgan's laws
	FilterTree_DeMorgan(&filter_tree);

	return filter_tree;
}
