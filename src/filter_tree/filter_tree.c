/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../value.h"
#include "filter_tree.h"
#include "../util/arr.h"
#include <assert.h>

static inline FT_FilterNode *LeftChild(const FT_FilterNode *node) {
	return node->cond.left;
}
static inline FT_FilterNode *RightChild(const FT_FilterNode *node) {
	return node->cond.right;
}

/* Return the negated operator of given op.
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
	case OP_CONTAINS:
		return OP_NOT_CONTAINS;
	case OP_NOT_CONTAINS:
		return OP_CONTAINS;
	case OP_STARTSWITH:
		return OP_NOT_STARTSWITH;
	case OP_NOT_STARTSWITH:
		return OP_STARTSWITH;
	case OP_ENDSWITH:
		return OP_NOT_ENDSWITH;
	case OP_NOT_ENDSWITH:
		return OP_ENDSWITH;
	default:
		assert(false);
	}
}

int IsNodePredicate(const FT_FilterNode *node) {
	return node->t == FT_N_PRED;
}

FT_FilterNode *AppendLeftChild(FT_FilterNode *root, FT_FilterNode *child) {
	root->cond.left = child;
	return root->cond.left;
}

FT_FilterNode *AppendRightChild(FT_FilterNode *root, FT_FilterNode *child) {
	root->cond.right = child;
	return root->cond.right;
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
	int rel;
	bool negate = false;
	SIValue argv[2];
	SIValue res;

	switch(op) {
	case OP_EQUAL:
	case OP_GT:
	case OP_GE:
	case OP_LT:
	case OP_LE:
	case OP_NEQUAL:
		rel = SIValue_Compare(*aVal, *bVal);
		/* Values are of disjoint types */
		if(rel == DISJOINT) {
			/* The filter passes if we're testing for inequality, and fails otherwise. */
			return (op == OP_NEQUAL);
		}

		switch(op) {
		case OP_EQUAL:
			return rel == 0;

		case OP_GT:
			return rel > 0;

		case OP_GE:
			return rel >= 0;

		case OP_LT:
			return rel < 0;

		case OP_LE:
			return rel <= 0;

		case OP_NEQUAL:
			return rel != 0;

		default:
			/* Op should be enforced by AST. */
			assert(0);
		}
		break;

	case OP_NOT_CONTAINS:
		negate = true;
	case OP_CONTAINS:
		// String matching.
		argv[0] = *aVal;
		argv[1] = *bVal;
		res = AR_CONTAINS(argv, 2);
		if(SI_TYPE(res) == T_NULL) return false;
		if(negate) return !res.longval;
		else return res.longval;
	case OP_NOT_STARTSWITH:
		negate = true;
	case OP_STARTSWITH:
		// String matching.
		argv[0] = *aVal;
		argv[1] = *bVal;
		res = AR_STARTSWITH(argv, 2);
		if(SI_TYPE(res) == T_NULL) return false;
		if(negate) return !res.longval;
		else return res.longval;
	case OP_NOT_ENDSWITH:
		negate = true;
	case OP_ENDSWITH:
		// String matching.
		argv[0] = *aVal;
		argv[1] = *bVal;
		res = AR_ENDSWITH(argv, 2);
		if(SI_TYPE(res) == T_NULL) return false;
		if(negate) return !res.longval;
		else return res.longval;
	default:
		break;
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
	/* Handle predicate node. */
	if(IsNodePredicate(root)) {
		return _applyPredicateFilters(root, r);
	}

	/* root->t == FT_N_COND, visit left subtree. */
	int pass = FilterTree_applyFilters(LeftChild(root), r);

	if(root->cond.op == OP_AND && pass == 1) {
		/* Visit right subtree. */
		pass *= FilterTree_applyFilters(RightChild(root), r);
	}

	if(root->cond.op == OP_OR && pass == 0) {
		/* Visit right subtree. */
		pass = FilterTree_applyFilters(RightChild(root), r);
	}

	return pass;
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
		AR_EXP_CollectEntityIDs(root->pred.lhs, modified);
		AR_EXP_CollectEntityIDs(root->pred.rhs, modified);
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

void _FilterTree_ApplyNegate(FT_FilterNode **root) {
	if((*root)->t == FT_N_COND) {
		if((*root)->cond.op == OP_NOT) {
			FilterTree_DeMorgan(root);
		} else {
			(*root)->cond.op = _NegateOperator((*root)->cond.op);
			_FilterTree_ApplyNegate(&(*root)->cond.left);
			_FilterTree_ApplyNegate(&(*root)->cond.right);
		}
	}

	if((*root)->t == FT_N_PRED) {
		(*root)->pred.op = _NegateOperator((*root)->cond.op);
		// TODO: should we negate arithmetic expressions?
	}
}

void FilterTree_DeMorgan(FT_FilterNode **root) {
	/* Search for NOT nodes and reduce using DeMorgan. */
	if(*root == NULL || (*root)->t == FT_N_PRED) return;

	// Node is of type condition.
	if((*root)->cond.op == OP_NOT) {
		assert((*root)->cond.right == NULL);
		_FilterTree_ApplyNegate(&(*root)->cond.left);
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

void _FilterTree_Print(const FT_FilterNode *root, int ident) {
	if(root == NULL) return;
	// Ident
	printf("%*s", ident, "");

	if(IsNodePredicate(root)) {
		char *left;
		AR_EXP_ToString(root->pred.lhs, &left);
		char *right;
		AR_EXP_ToString(root->pred.rhs, &right);
		printf("%s %d %s\n",  left, root->pred.op, right);
		free(left);
		free(right);
	} else {
		printf("%d\n", root->cond.op);
		_FilterTree_Print(LeftChild(root), ident + 4);
		_FilterTree_Print(RightChild(root), ident + 4);
	}
}

void FilterTree_Print(const FT_FilterNode *root) {
	if(root == NULL) {
		printf("empty filter tree\n");
		return;
	}
	_FilterTree_Print(root, 0);
}

void FilterTree_Free(FT_FilterNode *root) {
	if(root == NULL) return;
	if(IsNodePredicate(root)) {
		AR_EXP_Free(root->pred.lhs);
		AR_EXP_Free(root->pred.rhs);
	} else {
		FilterTree_Free(root->cond.left);
		FilterTree_Free(root->cond.right);
	}

	free(root);
}
