/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../value.h"
#include "filter_tree.h"
#include "../util/arr.h"
#include "../ast/ast_shared.h"
#include <assert.h>

/* forward declarations */
void _FilterTree_DeMorgan(FT_FilterNode **root, uint negate_count);

static inline FT_FilterNode *LeftChild(const FT_FilterNode *node) {
	return node->cond.left;
}
static inline FT_FilterNode *RightChild(const FT_FilterNode *node) {
	return node->cond.right;
}

/* Returns the negated operator of given op.
 * for example NOT(a > b) === a <= b */
static AST_Operator _NegateOperator(AST_Operator op) {
	switch(op) {
	case OP_AND:
		return OP_OR;
	case OP_OR:
		return OP_AND;
	case OP_EQUAL:
		return OP_NEQUAL;
	case OP_NEQUAL:
		return OP_EQUAL;
	case OP_LT:
		return OP_GE;
	case OP_GT:
		return OP_LE;
	case OP_LE:
		return OP_GT;
	case OP_GE:
		return OP_LT;
	default:
		assert(false);
	}
}

/* Negate expression by wrapping it with a NOT function, NOT(exp) */
static void _NegateExpression(AR_ExpNode **exp) {
	AR_ExpNode *root = AR_EXP_NewOpNode("not", 1);
	root->op.children[0] = *exp;
	*exp = root;
}

int IsNodePredicate(const FT_FilterNode *node) {
	return node->t == FT_N_PRED;
}

FT_FilterNode *FilterTree_AppendLeftChild(FT_FilterNode *root, FT_FilterNode *child) {
	root->cond.left = child;
	return root->cond.left;
}

FT_FilterNode *FilterTree_AppendRightChild(FT_FilterNode *root, FT_FilterNode *child) {
	root->cond.right = child;
	return root->cond.right;
}

FT_FilterNode *FilterTree_CreateExpressionFilter(AR_ExpNode *exp) {
	// TODO: make sure exp return boolean.
	assert(exp);

	FT_FilterNode *node = malloc(sizeof(FT_FilterNode));
	node->t = FT_N_EXP;
	node->exp.exp = exp;
	return node;
}

FT_FilterNode *FilterTree_CreatePredicateFilter(AST_Operator op, AR_ExpNode *lhs, AR_ExpNode *rhs) {
	FT_FilterNode *filterNode = malloc(sizeof(FT_FilterNode));
	filterNode->t = FT_N_PRED;
	filterNode->pred.op = op;
	filterNode->pred.lhs = lhs;
	filterNode->pred.rhs = rhs;
	return filterNode;
}

FT_FilterNode *FilterTree_CreateConditionFilter(AST_Operator op) {
	FT_FilterNode *filterNode = (FT_FilterNode *)malloc(sizeof(FT_FilterNode));
	filterNode->t = FT_N_COND;
	filterNode->cond.op = op;
	return filterNode;
}

void _FilterTree_SubTrees(const FT_FilterNode *root, Vector *sub_trees) {
	if(root == NULL) return;

	switch(root->t) {
	case FT_N_EXP:
	case FT_N_PRED:
		/* This is a simple predicate tree, can not traverse further. */
		Vector_Push(sub_trees, root);
		break;
	case FT_N_COND:
		switch(root->cond.op) {
		case OP_AND:
			/* Break AND down to its components. */
			_FilterTree_SubTrees(root->cond.left, sub_trees);
			_FilterTree_SubTrees(root->cond.right, sub_trees);
			free((FT_FilterNode *)root);
			break;
		case OP_OR:
			/* OR tree must be return as is. */
			Vector_Push(sub_trees, root);
			break;
		default:
			assert(0);
		}
		break;
	default:
		assert(0);
		break;
	}
}

Vector *FilterTree_SubTrees(const FT_FilterNode *root) {
	Vector *sub_trees = NewVector(FT_FilterNode *, 1);
	_FilterTree_SubTrees(root, sub_trees);
	return sub_trees;
}

/* Applies a single filter to a single result.
 * Compares given values, tests if values maintain desired relation (op) */
int _applyFilter(SIValue *aVal, SIValue *bVal, AST_Operator op) {
	int disjointOrNull = 0;
	int rel = SIValue_Compare(*aVal, *bVal, &disjointOrNull);
	// If there was null comparison, return false.
	if(disjointOrNull == COMPARED_NULL) return false;
	/* Values are of disjoint types */
	if(disjointOrNull == DISJOINT) {
		/* The filter passes if we're testing for inequality, and fails otherwise. */
		return (op == OP_NEQUAL);
	}

	switch(op) {
	case OP_EQUAL:
		return rel == 0;
	case OP_NEQUAL:
		return rel != 0;
	case OP_GT:
		return rel > 0;
	case OP_GE:
		return rel >= 0;
	case OP_LT:
		return rel < 0;
	case OP_LE:
		return rel <= 0;
	default:
		/* Op should be enforced by AST. */
		assert(0);
	}

	/* We shouldn't reach this point. */
	return 0;
}

int _applyPredicateFilters(const FT_FilterNode *root, const Record r) {
	/* A op B
	 * Evaluate the left and right sides of the predicate to obtain
	 * comparable SIValues. */
	SIValue lhs = AR_EXP_Evaluate(root->pred.lhs, r);
	SIValue rhs = AR_EXP_Evaluate(root->pred.rhs, r);

	int ret = _applyFilter(&lhs, &rhs, root->pred.op);

	SIValue_Free(&lhs);
	SIValue_Free(&rhs);

	return ret;
}

int FilterTree_applyFilters(const FT_FilterNode *root, const Record r) {
	switch(root->t) {
	case FT_N_COND: {
		/* root->t == FT_N_COND, visit left subtree. */
		int pass = FilterTree_applyFilters(LeftChild(root), r);

		if(root->cond.op == OP_AND && pass == 1) {
			/* Visit right subtree. */
			pass *= FilterTree_applyFilters(RightChild(root), r);
		} else if(root->cond.op == OP_OR && pass == 0) {
			/* Visit right subtree. */
			pass = FilterTree_applyFilters(RightChild(root), r);
		}

		return pass;
	}
	case FT_N_PRED: {
		return _applyPredicateFilters(root, r);
	}
	case FT_N_EXP: {
		SIValue res = AR_EXP_Evaluate(root->exp.exp, r);
		if(SIValue_IsNull(res)) {
			/* Expression evaluated to NULL should return false. */
			return FILTER_FAIL;
		} else if(SI_TYPE(res) & (SI_NUMERIC | T_BOOL)) {
			/* Numeric or Boolean evaluated to anuthing but 0
			* should return true. */
			if(SI_GET_NUMERIC(res) == 0) return FILTER_FAIL;
		}

		/* Boolean or Numeric != 0, String, Node, Edge, Ptr all evaluate to true. */
		return FILTER_PASS;
	}
	default:
		assert(false);
	}

	// We shouldn't be here.
	return 0;
}

void _FilterTree_CollectModified(const FT_FilterNode *root, rax *modified) {
	if(root == NULL) return;

	switch(root->t) {
	case FT_N_COND: {
		_FilterTree_CollectModified(root->cond.left, modified);
		_FilterTree_CollectModified(root->cond.right, modified);
		break;
	}
	case FT_N_PRED: {
		/* Traverse left and right-hand expressions, adding all encountered modified
		 * to the triemap.
		 * We'll typically encounter 0 or 1 modified in each expression,
		 * but there are multi-argument exceptions. */
		AR_EXP_CollectEntities(root->pred.lhs, modified);
		AR_EXP_CollectEntities(root->pred.rhs, modified);
		break;
	}
	case FT_N_EXP: {
		/* Traverse expression, adding all encountered modified to the triemap. */
		AR_EXP_CollectEntities(root->exp.exp, modified);
		break;
	}
	default: {
		assert(0);
		break;
	}
	}
}

rax *FilterTree_CollectModified(const FT_FilterNode *root) {
	rax *modified = raxNew();
	_FilterTree_CollectModified(root, modified);

	return modified;
}

void _FilterTree_CollectAttributes(const FT_FilterNode *root, rax *attributes) {
	if(root == NULL) return;

	switch(root->t) {
	case FT_N_COND: {
		_FilterTree_CollectAttributes(root->cond.left, attributes);
		_FilterTree_CollectAttributes(root->cond.right, attributes);
		break;
	}
	case FT_N_PRED: {
		/* Traverse left and right-hand expressions, adding all encountered attributes
		* to the triemap. */
		AR_EXP_CollectAttributes(root->pred.lhs, attributes);
		AR_EXP_CollectAttributes(root->pred.rhs, attributes);
		break;
	}
	case FT_N_EXP: {
		AR_EXP_CollectAttributes(root->exp.exp, attributes);
		break;
	}
	default: {
		assert(0);
	}
	}
}

rax *FilterTree_CollectAttributes(const FT_FilterNode *root) {
	rax *attributes = raxNew();
	_FilterTree_CollectAttributes(root, attributes);
	return attributes;
}

bool FilterTree_containsOp(const FT_FilterNode *root, AST_Operator op) {
	switch(root->t) {
	case FT_N_COND:
		if(FilterTree_containsOp(root->cond.left, op)) return true;
		if(FilterTree_containsOp(root->cond.right, op)) return true;
		return false;
	case FT_N_EXP:
		return false;
	case FT_N_PRED:
		return (root->pred.op == op);
	default:
		assert(false);
		return false;
	}
}

bool FilterTree_containsFunc(const FT_FilterNode *root, const char *func, FT_FilterNode **node) {
	if(root == NULL) return false;
	switch(root->t) {
	case FT_N_COND: {
		return FilterTree_containsFunc(root->cond.left, func, node) ||
			   FilterTree_containsFunc(root->cond.right, func, node);
	}
	case FT_N_PRED: {
		if(AR_EXP_ContainsFunc(root->pred.lhs, func) || AR_EXP_ContainsFunc(root->pred.rhs, func)) {
			*node = (FT_FilterNode *)root;
			return true;
		}
		return false;
	}
	case FT_N_EXP: {
		if(AR_EXP_ContainsFunc(root->exp.exp, func)) {
			*node = (FT_FilterNode *) root;
			return true;
		}
		return false;
	}
	default:
		assert(false);
	}
	return false;
}

void _FilterTree_ApplyNegate(FT_FilterNode **root, uint negate_count) {
	switch((*root)->t) {
	case FT_N_EXP:
		if(negate_count % 2 == 1) {
			_NegateExpression(&((*root)->exp.exp));
		}
		break;
	case FT_N_PRED:
		if(negate_count % 2 == 1) {
			(*root)->pred.op = _NegateOperator((*root)->cond.op);
		}
		break;
	case FT_N_COND:
		if((*root)->cond.op == OP_NOT) {
			// _FilterTree_DeMorgan will increase negate_count by 1.
			_FilterTree_DeMorgan(root, negate_count);
		} else {
			if(negate_count % 2 == 1) {
				(*root)->cond.op = _NegateOperator((*root)->cond.op);
			}
			_FilterTree_ApplyNegate(&(*root)->cond.left, negate_count);
			_FilterTree_ApplyNegate(&(*root)->cond.right, negate_count);
		}
		break;
	default:
		assert(false);
	}
}

bool FilterTree_Valid(const FT_FilterNode *root) {
	// An empty tree is has a valid structure.
	if(!root) return true;

	switch(root->t) {
	case FT_N_EXP:
		// No need to validate expression.
		return true;
		break;
	case FT_N_PRED:
		// Empty or semi empty predicate, invalid structure.
		if((!root->pred.lhs || !root->pred.rhs)) return false;
		break;
	case FT_N_COND:
		// Empty condition, invalid structure.
		// OR, AND should utilize both left and right children
		// NOT utilize only the left child.
		if(!root->cond.left && !root->cond.right) return false;
		if(!FilterTree_Valid(root->cond.left)) return false;
		if(!FilterTree_Valid(root->cond.right)) return false;
		break;
	default:
		assert("Unknown filter tree node" && false);
	}
	return true;
}

void _FilterTree_DeMorgan(FT_FilterNode **root, uint negate_count) {
	/* Search for NOT nodes and reduce using DeMorgan. */
	if(*root == NULL || (*root)->t == FT_N_PRED || (*root)->t == FT_N_EXP) return;

	// Node is of type condition.
	if((*root)->cond.op == OP_NOT) {
		assert((*root)->cond.right == NULL);
		_FilterTree_ApplyNegate(&(*root)->cond.left, negate_count + 1);
		// Replace NOT node with only child
		FT_FilterNode *child = (*root)->cond.left;
		(*root)->cond.left = NULL;
		FilterTree_Free(*root);
		*root = child;
	} else {
		FilterTree_DeMorgan(&((*root)->cond.left));
		FilterTree_DeMorgan(&((*root)->cond.right));
	}
}

void FilterTree_DeMorgan(FT_FilterNode **root) {
	_FilterTree_DeMorgan(root, 0);
}

void _FilterTree_Print(const FT_FilterNode *root, int ident) {
	char *exp = NULL;
	char *left = NULL;
	char *right = NULL;

	if(root == NULL) return;
	// Ident
	printf("%*s", ident, "");

	switch(root->t) {
	case FT_N_EXP:
		AR_EXP_ToString(root->exp.exp, &exp);
		printf("%s\n",  exp);
		rm_free(exp);
		break;
	case FT_N_PRED:
		AR_EXP_ToString(root->pred.lhs, &left);
		AR_EXP_ToString(root->pred.rhs, &right);
		printf("%s %d %s\n",  left, root->pred.op, right);
		rm_free(left);
		rm_free(right);
		break;
	case FT_N_COND:
		printf("%d\n", root->cond.op);
		_FilterTree_Print(LeftChild(root), ident + 4);
		_FilterTree_Print(RightChild(root), ident + 4);
		break;
	default:
		assert(false);
	}
}

void FilterTree_Print(const FT_FilterNode *root) {
	if(root == NULL) {
		printf("empty filter tree\n");
		return;
	}
	_FilterTree_Print(root, 0);
}

FT_FilterNode *FilterTree_Clone(const FT_FilterNode *root) {
	if(root == NULL) return NULL;
	FT_FilterNode *new_root;
	switch(root->t) {
	case FT_N_EXP: {
		new_root = FilterTree_CreateExpressionFilter(root->exp.exp);
		break;
	}
	case FT_N_PRED: {
		AR_ExpNode *lhs = AR_EXP_Clone(root->pred.lhs);
		AR_ExpNode *rhs = AR_EXP_Clone(root->pred.rhs);
		new_root = FilterTree_CreatePredicateFilter(root->pred.op, lhs, rhs);
		break;
	}

	case FT_N_COND: {
		new_root = FilterTree_CreateConditionFilter(root->cond.op);
		if(root->cond.left) AppendLeftChild(new_root, FilterTree_Clone(root->cond.left));
		if(root->cond.right) AppendLeftChild(new_root, FilterTree_Clone(root->cond.right));
		break;
	}
	default:
		assert(false);
	}

	return new_root;
}

void FilterTree_Free(FT_FilterNode *root) {
	if(root == NULL) return;
	switch(root->t) {
	case FT_N_EXP:
		AR_EXP_Free(root->exp.exp);
		break;
	case FT_N_PRED:
		AR_EXP_Free(root->pred.lhs);
		AR_EXP_Free(root->pred.rhs);
		break;
	case FT_N_COND:
		FilterTree_Free(root->cond.left);
		FilterTree_Free(root->cond.right);
		break;
	default:
		assert(false);
	}

	free(root);
}
