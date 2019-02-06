/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <assert.h>
#include "../value.h"
#include "filter_tree.h"
#include "../parser/grammar.h"
#include "../query_executor.h"
#include "../util/vector.h"

FT_FilterNode* LeftChild(const FT_FilterNode *node) { return node->cond.left; }
FT_FilterNode* RightChild(const FT_FilterNode *node) { return node->cond.right; }

int IsNodePredicate(const FT_FilterNode *node) {
    return node->t == FT_N_PRED;
}

FT_FilterNode* CreateCondFilterNode(int op) {
    FT_FilterNode* filterNode = (FT_FilterNode*)malloc(sizeof(FT_FilterNode));
    filterNode->t = FT_N_COND;
    filterNode->cond.op = op;
    return filterNode;
}

FT_FilterNode* _CreatePredicateFilterNode(const AST *ast, const AST_PredicateNode *pn) {
    FT_FilterNode *filterNode = malloc(sizeof(FT_FilterNode));
    filterNode->t= FT_N_PRED;
    filterNode->pred.op = pn->op;
    filterNode->pred.lhs = AR_EXP_BuildFromAST(ast, pn->lhs);
    filterNode->pred.rhs = AR_EXP_BuildFromAST(ast, pn->rhs);
    return filterNode;
}

FT_FilterNode *AppendLeftChild(FT_FilterNode *root, FT_FilterNode *child) {
    root->cond.left = child;
    return root->cond.left;
}

FT_FilterNode *AppendRightChild(FT_FilterNode *root, FT_FilterNode *child) {
    root->cond.right = child;
    return root->cond.right;
}

void _FilterTree_SubTrees(const FT_FilterNode *root, Vector *sub_trees) {
    if (root == NULL) return;

    switch(root->t) {
        case FT_N_PRED:
            /* This is a simple predicate tree, can not traverse further. */
            Vector_Push(sub_trees, root);
            break;
        case FT_N_COND:
            switch(root->cond.op) {
                case AND:
                    /* Break AND down to its components. */
                    _FilterTree_SubTrees(root->cond.left, sub_trees);
                    _FilterTree_SubTrees(root->cond.right, sub_trees);
                    break;
                case OR:
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

Vector* FilterTree_SubTrees(const FT_FilterNode *root) {
    Vector *sub_trees = NewVector(FT_FilterNode *, 1);
    _FilterTree_SubTrees(root, sub_trees);
    return sub_trees;
}

FT_FilterNode* BuildFiltersTree(const AST *ast, const AST_FilterNode *root) {
    FT_FilterNode *filterNode;

    if(root->t == N_PRED) {
        filterNode = _CreatePredicateFilterNode(ast, &root->pn);
    } else {
        filterNode = CreateCondFilterNode(root->cn.op);
        AppendLeftChild(filterNode, BuildFiltersTree(ast, root->cn.left));
        AppendRightChild(filterNode, BuildFiltersTree(ast, root->cn.right));
    }

    return filterNode;
}

/* Applies a single filter to a single result.
 * Compares given values, tests if values maintain desired relation (op) */
int _applyFilter(SIValue* aVal, SIValue* bVal, int op) {
    int rel = SIValue_Compare(*aVal, *bVal);
    /* Values are of disjoint types */
    if (rel == DISJOINT) {
        /* The filter passes if we're testing for inequality, and fails otherwise.*/
        if (op == NE) return 1;
        return 0;
    }

    switch(op) {
        case EQ:
        return rel == 0;

        case GT:
        return rel > 0;

        case GE:
        return rel >= 0;

        case LT:
        return rel < 0;

        case LE:
        return rel <= 0;

        case NE:
        return rel != 0;

        default:
        /* Op should be enforced by AST. */
        assert(0);
    }
    /* We shouldn't reach this point. */
    return 0;
}

int _applyPredicateFilters(const FT_FilterNode* root, const Record r) {
    /* A op B
     * Evaluate the left and right sides of the predicate to obtain
     * comparable SIValues. */
    SIValue lhs = AR_EXP_Evaluate(root->pred.lhs, r);
    SIValue rhs = AR_EXP_Evaluate(root->pred.rhs, r);

    return _applyFilter(&lhs, &rhs, root->pred.op);
}

int FilterTree_applyFilters(const FT_FilterNode* root, const Record r) {
    /* Handle predicate node. */
    if(IsNodePredicate(root)) {
        return _applyPredicateFilters(root, r);
    }

    /* root->t == FT_N_COND, visit left subtree. */
    int pass = FilterTree_applyFilters(LeftChild(root), r);
    
    if(root->cond.op == AND && pass == 1) {
        /* Visit right subtree. */
        pass *= FilterTree_applyFilters(RightChild(root), r);
    }

    if(root->cond.op == OR && pass == 0) {
        /* Visit right subtree. */
        pass = FilterTree_applyFilters(RightChild(root), r);
    }

    return pass;
}

void _FilterTree_CollectAliases(const FT_FilterNode *root, TrieMap *aliases) {
    if(root == NULL) return;

    switch(root->t) {
        case FT_N_COND:
        {
            _FilterTree_CollectAliases(root->cond.left, aliases);
            _FilterTree_CollectAliases(root->cond.right, aliases);
            break;
        }
        case FT_N_PRED:
        {
            /* Traverse left and right-hand expressions, adding all encountered aliases
             * to the triemap.
             * We'll typically encounter 0 or 1 aliases in each expression,
             * but there are multi-argument exceptions. */
            AR_EXP_CollectAliases(root->pred.lhs, aliases);
            AR_EXP_CollectAliases(root->pred.rhs, aliases);

            break;
        }
        default:
        {
            assert(0);
            break;
        }
    }
}

Vector *FilterTree_CollectAliases(const FT_FilterNode *root) {
    TrieMap *t = NewTrieMap();
    _FilterTree_CollectAliases(root, t);

    Vector *aliases = NewVector(char*, t->cardinality);
    TrieMapIterator *it = TrieMap_Iterate(t, "", 0);
    
    char *ptr;
    tm_len_t len;
    void *value;

    while(TrieMapIterator_Next(it, &ptr, &len, &value)) {
        char *alias = strdup(ptr);
        alias[len] = 0;
        Vector_Push(aliases, alias);
    }

    TrieMapIterator_Free(it);
    TrieMap_Free(t, NULL);
    return aliases;
}

void _FilterTree_Print(const FT_FilterNode *root, int ident) {
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
        _FilterTree_Print(LeftChild(root), ident+4);
        _FilterTree_Print(RightChild(root), ident+4);
    }
}

void FilterTree_Print(const FT_FilterNode *root) {
    if(root == NULL) {
        printf("empty filter tree\n");
        return; 
    }
    _FilterTree_Print(root, 0);
}

void _FilterTree_FreePredNode(FT_PredicateNode node) {
    AR_EXP_Free(node.lhs);
    AR_EXP_Free(node.rhs);

    // TODO free node itself 
}

void FilterTree_Free(FT_FilterNode *root) {
    if(root == NULL) { return; }
    if(IsNodePredicate(root)) {
        _FilterTree_FreePredNode(root->pred);
    } else {
        FilterTree_Free(root->cond.left);
        FilterTree_Free(root->cond.right);
    }

    free(root);
}
