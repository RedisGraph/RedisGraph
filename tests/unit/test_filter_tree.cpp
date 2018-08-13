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
#include "../../src/filter_tree/filter_tree.h"

#ifdef __cplusplus
}
#endif

class FilterTreeTest: public ::testing::Test {
    protected:

    FT_FilterNode* _build_simple_const_tree() {
        SIValue value = SI_DoubleVal(34);
        AST_FilterNode *root = New_AST_ConstantPredicateNode("me", "age", EQ, value);
        FT_FilterNode *tree = BuildFiltersTree(root);
        Free_AST_FilterNode(root);
        return tree;
    }

    FT_FilterNode* _build_simple_varying_tree() {
        AST_FilterNode *root = New_AST_VaryingPredicateNode("me", "age", GT, "him", "age");
        FT_FilterNode *tree = BuildFiltersTree(root);
        Free_AST_FilterNode(root);
        return tree;
    }

    FT_FilterNode* _build_cond_tree(int cond) {
        SIValue age = SI_DoubleVal(34);
        SIValue height = SI_DoubleVal(188);
        AST_FilterNode *left = New_AST_ConstantPredicateNode("me", "age", GT, age);
        AST_FilterNode *right = New_AST_ConstantPredicateNode("me", "height", LE, height);
        AST_FilterNode *root = New_AST_ConditionNode(left, cond, right);
        FT_FilterNode *tree = BuildFiltersTree(root);
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
        SIValue age = SI_DoubleVal(34);
        SIValue height = SI_DoubleVal(188);

        AST_FilterNode *left_left_child = New_AST_ConstantPredicateNode("me", "age", GT, age);
        AST_FilterNode *left_right_child = New_AST_ConstantPredicateNode("he", "height", LE, height);
        AST_FilterNode *left = New_AST_ConditionNode(left_left_child, AND, left_right_child);

        AST_FilterNode *right_left_child = New_AST_ConstantPredicateNode("she", "age", GT, age);
        AST_FilterNode *right_right_child = New_AST_ConstantPredicateNode("theirs", "height", LE, height);
        AST_FilterNode *right = New_AST_ConditionNode(right_left_child, OR, right_right_child);

        AST_FilterNode *root = New_AST_ConditionNode(left, AND, right);
        FT_FilterNode *tree = BuildFiltersTree(root);

        Free_AST_FilterNode(root);
        return tree;
    }

    void _compareFilterTreeVaryingNode(const FT_FilterNode *a, const FT_FilterNode *b) {
        EXPECT_EQ(a->t, b->t);
        EXPECT_EQ(a->pred.op, b->pred.op);
        EXPECT_EQ(a->pred.t, b->pred.t);
        EXPECT_EQ(a->pred.cf, b->pred.cf);
        EXPECT_EQ(strcmp(a->pred.Lop.alias, b->pred.Lop.alias), 0);
        EXPECT_EQ(strcmp(a->pred.Lop.property, b->pred.Lop.property), 0);
        EXPECT_EQ(strcmp(a->pred.Rop.alias, b->pred.Rop.alias), 0);
        EXPECT_EQ(strcmp(a->pred.Rop.property, b->pred.Rop.property), 0);
    }

    void _compareFilterTreeConstNode(const FT_FilterNode *a, const FT_FilterNode *b) {
        EXPECT_EQ(a->t, b->t);
        EXPECT_EQ(a->pred.op, b->pred.op);
        EXPECT_EQ(a->pred.t, b->pred.t);
        EXPECT_EQ(a->pred.cf, b->pred.cf);
        EXPECT_EQ(strcmp(a->pred.Lop.alias, b->pred.Lop.alias), 0);
        EXPECT_EQ(strcmp(a->pred.Lop.property, b->pred.Lop.property), 0);
        EXPECT_EQ(a->pred.constVal.type, b->pred.constVal.type);
    }

    void compareFilterTrees(const FT_FilterNode *a, const FT_FilterNode *b) {
        EXPECT_EQ(a->t, b->t);
        
        if(a->t == FT_N_PRED) {
            if(a->pred.t == FT_N_CONSTANT) {
                _compareFilterTreeConstNode(a, b);
            } else {
                _compareFilterTreeVaryingNode(a, b);
            }
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
