#include "ast_build_filter_tree.h"
#include "ast_shared.h"
#include "../util/arr.h"

// Forward declaration
FT_FilterNode *_FilterNode_FromAST(const cypher_astnode_t *expr);

FT_FilterNode *_CreatePredicateFilterNode(AST_Operator op, const cypher_astnode_t *lhs,
										  const cypher_astnode_t *rhs) {
	return FilterTree_CreatePredicateFilter(op, AR_EXP_FromExpression(lhs), AR_EXP_FromExpression(rhs));
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

FT_FilterNode *_CreateFilterSubtree(AST_Operator op, const cypher_astnode_t *lhs,
									const cypher_astnode_t *rhs) {
	FT_FilterNode *filter = NULL;
	switch(op) {
	case OP_OR:
	case OP_AND:
		filter = FilterTree_CreateConditionFilter(op);
		AppendLeftChild(filter, _FilterNode_FromAST(record_map, lhs));
		AppendRightChild(filter, _FilterNode_FromAST(record_map, rhs));
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
FT_FilterNode *_convertBinaryOperator(const cypher_astnode_t *op_node) {
	const cypher_operator_t *operator = cypher_ast_binary_operator_get_operator(op_node);
	// Arguments are of type CYPHER_AST_EXPRESSION
	const cypher_astnode_t *lhs = cypher_ast_binary_operator_get_argument1(op_node);
	const cypher_astnode_t *rhs = cypher_ast_binary_operator_get_argument2(op_node);

	AST_Operator op = AST_ConvertOperatorNode(operator);

	return _CreateFilterSubtree(op, lhs, rhs);
}

/* A comparison node contains two arrays - one of operators, and one of expressions.
 * Most comparisons will only have one operator and two expressions, but Cypher
 * allows more complex formulations like "x < y <= z".
 * A comparison takes a form such as "WHERE a.val < y.val". */
FT_FilterNode *_convertComparison(const cypher_astnode_t *comparison_node) {
	uint nelems = cypher_ast_comparison_get_length(comparison_node);
	assert(nelems == 1); // TODO Cypher comparisons are allowed to be longer, as in "x < y <= z"

	const cypher_operator_t *operator = cypher_ast_comparison_get_operator(comparison_node, 0);
	AST_Operator op = AST_ConvertOperatorNode(operator);

	// All arguments are of type CYPHER_AST_EXPRESSION
	const cypher_astnode_t *lhs = cypher_ast_comparison_get_argument(comparison_node, 0);
	const cypher_astnode_t *rhs = cypher_ast_comparison_get_argument(comparison_node, 1);

	return _CreatePredicateFilterNode(op, lhs, rhs);
}

static FT_FilterNode *_convertInlinedProperties(const AST *ast, const cypher_astnode_t *entity,
												EntityType type) {
	const cypher_astnode_t *props = NULL;

	if(type == ENTITY_NODE) {
		props = cypher_ast_node_pattern_get_properties(entity);
	} else { // relation
		props = cypher_ast_rel_pattern_get_properties(entity);
	}

	if(!props) return NULL;

	FT_FilterNode *root = NULL;
	uint nelems = cypher_ast_map_nentries(props);
	for(uint i = 0; i < nelems; i ++) {
		// key is of type CYPHER_AST_PROP_NAME
		const cypher_astnode_t *prop_node = cypher_ast_map_get_key(props, i);
		const char *var = cypher_ast_property_operator_get_prop_name(prop_node);
		const char *prop = cypher_ast_prop_name_get_value(prop_node);
		AR_ExpNode *lhs = AR_EXP_NewVariableOperandNode(var, prop);

		// val is of type CYPHER_AST_EXPRESSION
		const cypher_astnode_t *val = cypher_ast_map_get_value(props, i);
		AR_ExpNode *rhs = AR_EXP_FromExpression(record_map, val);
		/* TODO In a query like:
		 * "MATCH (r:person {name:"Roi"}) RETURN r"
		 * (note the repeated double quotes) - this creates a variable rather than a scalar.
		 * Can we use this to handle escape characters or something? How does it work? */
		FT_FilterNode *t = FilterTree_CreatePredicateFilter(OP_EQUAL, lhs, rhs);
		_FT_Append(&root, t);
	}
	return root;
}

FT_FilterNode *_FilterNode_FromAST(const cypher_astnode_t *expr) {
	assert(expr);
	cypher_astnode_type_t type = cypher_astnode_type(expr);
	if(type == CYPHER_AST_BINARY_OPERATOR) {
		return _convertBinaryOperator(expr);
	} else if(type == CYPHER_AST_COMPARISON) {
		return _convertComparison(expr);
	} else if(type == CYPHER_AST_APPLY_OPERATOR) {
		assert(false && "Unary APPLY operators are not currently supported in filters.");
	}
	assert(false);
	return NULL;
}

void _AST_ConvertFilters(const AST *ast, FT_FilterNode **root, const cypher_astnode_t *entity) {
	if(!entity) return;

	cypher_astnode_type_t type = cypher_astnode_type(entity);

	FT_FilterNode *node = NULL;
	// If the current entity is a node or edge pattern, capture its properties map (if any)
	if(type == CYPHER_AST_NODE_PATTERN) {
		node = _convertInlinedProperties(ast, entity, ENTITY_NODE);
	} else if(type == CYPHER_AST_REL_PATTERN) {
		node = _convertInlinedProperties(ast, entity, ENTITY_EDGE);
	} else if(type == CYPHER_AST_COMPARISON) {
		node = _convertComparison(entity);
	} else if(type == CYPHER_AST_BINARY_OPERATOR) {
		node = _convertBinaryOperator(entity);
	} else if(type == CYPHER_AST_UNARY_OPERATOR) {
		// TODO, also n-ary maybe
		assert(false && "unary filters are not currently supported.");
	} else if(type == CYPHER_AST_APPLY_OPERATOR) {
		assert(false && "APPLY operators are not currently supported in filters.");
	} else {
		uint child_count = cypher_astnode_nchildren(entity);
		for(uint i = 0; i < child_count; i++) {
			const cypher_astnode_t *child = cypher_astnode_get_child(entity, i);
			// Recursively continue searching
			_AST_ConvertFilters(ast, root, child);
		}
	}
	if(node) _FT_Append(root, node);
}

FT_FilterNode *AST_BuildFilterTree(AST *ast) {
	FT_FilterNode *filter_tree = NULL;
	const cypher_astnode_t **match_clauses = AST_GetClauses(ast, CYPHER_AST_MATCH);
	if(match_clauses) {
		uint match_count = array_len(match_clauses);
		for(uint i = 0; i < match_count; i ++) {
			_AST_ConvertFilters(ast, &filter_tree, match_clauses[i]);
		}
		array_free(match_clauses);
	}

	const cypher_astnode_t **merge_clauses = AST_GetClauses(ast, CYPHER_AST_MERGE);
	if(merge_clauses) {
		uint merge_count = array_len(merge_clauses);
		for(uint i = 0; i < merge_count; i ++) {
			_AST_ConvertFilters(ast, &filter_tree, merge_clauses[i]);
		}
		array_free(merge_clauses);
	}

	return filter_tree;
}
