/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "../../deps/googletest/include/gtest/gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include "../../src/rmutil/vector.h"
#include "../../src/parser/grammar.h"
#include "../../src/parser/ast_arithmetic_expression.h"
#include "../../src/filter_tree/filter_tree.h"

#ifdef __cplusplus
}
#endif

class FilterTreeTest: public ::testing::Test {
    protected:

    FT_FilterNode* _build_simple_const_tree() {
        SIValue value = SI_DoubleVal(34);
        AST_ArithmeticExpressionNode *lhs = New_AST_AR_EXP_VariableOperandNode("me", "age");
        AST_ArithmeticExpressionNode *rhs = New_AST_AR_EXP_ConstOperandNode(value);

        AST_FilterNode *root = New_AST_PredicateNode(lhs, EQ, rhs);
        FT_FilterNode *tree = BuildFiltersTree(root, NULL);

        Free_AST_ArithmeticExpressionNode(lhs);
        Free_AST_ArithmeticExpressionNode(rhs);
        Free_AST_FilterNode(root);

        return tree;
    }

    FT_FilterNode* _build_simple_varying_tree() {
        AST_ArithmeticExpressionNode *lhs = New_AST_AR_EXP_VariableOperandNode("me", "age");
        AST_ArithmeticExpressionNode *rhs = New_AST_AR_EXP_VariableOperandNode("him", "age");
        AST_FilterNode *root = New_AST_PredicateNode(lhs, GT, rhs);
        FT_FilterNode *tree = BuildFiltersTree(root, NULL);

        Free_AST_ArithmeticExpressionNode(lhs);
        Free_AST_ArithmeticExpressionNode(rhs);
        Free_AST_FilterNode(root);

        return tree;
    }

    FT_FilterNode* _build_cond_tree(int cond) {
        AST_ArithmeticExpressionNode *left_lhs = New_AST_AR_EXP_VariableOperandNode("me", "age");
        SIValue age = SI_DoubleVal(34);
        AST_ArithmeticExpressionNode *left_rhs = New_AST_AR_EXP_ConstOperandNode(age);
        AST_FilterNode *left = New_AST_PredicateNode(left_lhs, GT, left_rhs);

        AST_ArithmeticExpressionNode *right_lhs = New_AST_AR_EXP_VariableOperandNode("me", "height");
        SIValue height = SI_DoubleVal(188);
        AST_ArithmeticExpressionNode *right_rhs = New_AST_AR_EXP_ConstOperandNode(height);
        AST_FilterNode *right = New_AST_PredicateNode(right_lhs, LE, right_rhs);

        AST_FilterNode *root = New_AST_ConditionNode(left, cond, right);
        FT_FilterNode *tree = BuildFiltersTree(root, NULL);

        Free_AST_ArithmeticExpressionNode(left_lhs);
        Free_AST_ArithmeticExpressionNode(left_rhs);
        Free_AST_ArithmeticExpressionNode(right_lhs);
        Free_AST_ArithmeticExpressionNode(right_rhs);
        Free_AST_FilterNode(root);

        return tree;
    }

    FT_FilterNode* _build_AND_cond_tree() {
        return _build_cond_tree(AND);
    }

    FT_FilterNode* _build_OR_cond_tree() {
        return _build_cond_tree(OR);
    }

    FT_FilterNode* _build_deep_tree() {
        /* Build a tree with an AND at the root
        * AND as a left child
        * OR as a right child */

        // Build out the variadic nodes to use in filters
        AST_ArithmeticExpressionNode *left_left_variable = New_AST_AR_EXP_VariableOperandNode("me", "age");
        AST_ArithmeticExpressionNode *left_right_variable = New_AST_AR_EXP_VariableOperandNode("he", "height");
        AST_ArithmeticExpressionNode *right_left_variable = New_AST_AR_EXP_VariableOperandNode("she", "age");
        AST_ArithmeticExpressionNode *right_right_variable = New_AST_AR_EXP_VariableOperandNode("theirs", "height");

        // Build out the constant nodes to filter against
        SIValue age = SI_DoubleVal(34);
        AST_ArithmeticExpressionNode *age_expression = New_AST_AR_EXP_ConstOperandNode(age);
        SIValue height = SI_DoubleVal(188);
        AST_ArithmeticExpressionNode *height_expression = New_AST_AR_EXP_ConstOperandNode(height);
        // Build the AST filter nodes
        AST_FilterNode *left_left_child = New_AST_PredicateNode(left_left_variable, GT, age_expression);
        AST_FilterNode *left_right_child = New_AST_PredicateNode(left_right_variable, LE, height_expression);
        AST_FilterNode *left = New_AST_ConditionNode(left_left_child, AND, left_right_child);

        AST_FilterNode *right_left_child = New_AST_PredicateNode(right_left_variable, GT, age_expression);
        AST_FilterNode *right_right_child = New_AST_PredicateNode(right_right_variable, LE, height_expression);
        AST_FilterNode *right = New_AST_ConditionNode(right_left_child, OR, right_right_child);

        AST_FilterNode *root = New_AST_ConditionNode(left, AND, right);
        FT_FilterNode *tree = BuildFiltersTree(root, NULL);

        Free_AST_ArithmeticExpressionNode(age_expression);
        Free_AST_ArithmeticExpressionNode(height_expression);
        Free_AST_FilterNode(root);

        return tree;
    }

    void _compareFilterTreePredicateNode(const FT_FilterNode *a, const FT_FilterNode *b) {
        EXPECT_EQ(a->t, b->t);
        EXPECT_EQ(a->t, FT_N_PRED);
        EXPECT_EQ(a->pred.op, b->pred.op);

        char *a_variable;
        char *b_variable;
        AR_EXP_ToString(a->pred.lhs, &a_variable);
        AR_EXP_ToString(b->pred.lhs, &b_variable);
        EXPECT_STREQ(a_variable, b_variable);

        free(a_variable);
        free(b_variable);

        AR_EXP_ToString(a->pred.rhs, &a_variable);
        AR_EXP_ToString(b->pred.rhs, &b_variable);
        EXPECT_STREQ(a_variable, b_variable);

        free(a_variable);
        free(b_variable);
    }

    void compareFilterTrees(const FT_FilterNode *a, const FT_FilterNode *b) {
        EXPECT_EQ(a->t, b->t);
        
        if(a->t == FT_N_PRED) {
            _compareFilterTreePredicateNode(a, b);
        } else {
            EXPECT_EQ(a->cond.op, b->cond.op);
            compareFilterTrees(a->cond.left, b->cond.left);
            compareFilterTrees(a->cond.right, b->cond.right);
        }
    }
};


TEST_F(FilterTreeTest, SubTrees) {
    FT_FilterNode *tree = _build_simple_const_tree();
    Vector *sub_trees = FilterTree_SubTrees(tree);
    EXPECT_EQ(Vector_Size(sub_trees), 1);

    FT_FilterNode *sub_tree;
    Vector_Get(sub_trees, 0, &sub_tree);
    compareFilterTrees(tree, sub_tree);

    Vector_Free(sub_trees);
    FilterTree_Free(tree);

    //------------------------------------------------------------------------------

    tree = _build_simple_varying_tree();
    sub_trees = FilterTree_SubTrees(tree);
    EXPECT_EQ(Vector_Size(sub_trees), 1);

    Vector_Get(sub_trees, 0, &sub_tree);
    compareFilterTrees(tree, sub_tree);

    Vector_Free(sub_trees);
    FilterTree_Free(tree);

    //------------------------------------------------------------------------------

    tree = _build_AND_cond_tree();
    sub_trees = FilterTree_SubTrees(tree);
    EXPECT_EQ(Vector_Size(sub_trees), 2);

    Vector_Get(sub_trees, 0, &sub_tree);    
    compareFilterTrees(tree->cond.left, sub_tree);
    
    Vector_Get(sub_trees, 1, &sub_tree);
    compareFilterTrees(tree->cond.right, sub_tree);

    Vector_Free(sub_trees);
    FilterTree_Free(tree);

    //------------------------------------------------------------------------------
    
    tree = _build_OR_cond_tree();
    sub_trees = FilterTree_SubTrees(tree);
    EXPECT_EQ(Vector_Size(sub_trees), 1);

    Vector_Get(sub_trees, 0, &sub_tree);
    compareFilterTrees(tree, sub_tree);

    Vector_Free(sub_trees);
    FilterTree_Free(tree);

    //------------------------------------------------------------------------------

    tree = _build_deep_tree();
    sub_trees = FilterTree_SubTrees(tree);
    EXPECT_EQ(Vector_Size(sub_trees), 3);

    Vector_Get(sub_trees, 0, &sub_tree);
    compareFilterTrees(tree->cond.left->cond.left, sub_tree);

    Vector_Get(sub_trees, 1, &sub_tree);
    compareFilterTrees(tree->cond.left->cond.right, sub_tree);

    Vector_Get(sub_trees, 2, &sub_tree);
    compareFilterTrees(tree->cond.right, sub_tree);

    Vector_Free(sub_trees);
    FilterTree_Free(tree);
}

TEST_F(FilterTreeTest, CollectAliases) {
    FT_FilterNode *tree = _build_deep_tree();
    Vector *aliases = FilterTree_CollectAliases(tree);
    EXPECT_EQ(Vector_Size(aliases), 4);
    
    char *alias;
    const char *expectation[4] = {"me", "he", "she", "theirs"};
    for(int i = 0; i < 4; i++) {
        const char *expected = expectation[i];
        int j = 0;
        for(; j < Vector_Size(aliases); j++) {
            Vector_Get(aliases, j, &alias);
            if(!strcmp(alias, expected)) break;
        }
        EXPECT_NE(j, 4);
    }

    /* Clean up. */
    for(int i = 0; i < Vector_Size(aliases); i++) {
        Vector_Get(aliases, i, &alias);
        free(alias);
    }
    Vector_Free(aliases);
    FilterTree_Free(tree);
}
