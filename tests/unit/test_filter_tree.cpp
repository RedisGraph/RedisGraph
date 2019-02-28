/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../../deps/googletest/include/gtest/gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include "../../src/util/vector.h"
#include "../../src/parser/ast.h"
#include "../../src/parser/grammar.h"
#include "../../src/parser/ast_arithmetic_expression.h"
#include "../../src/filter_tree/filter_tree.h"
#include "../../src/util/arr.h"
#include "../../src/util/rmalloc.h"
#include "../../src/query_executor.h"

#ifdef __cplusplus
}
#endif

extern pthread_key_t _tlsGCKey;    // Thread local storage graph context key.

class FilterTreeTest: public ::testing::Test {
    protected:

    static void SetUpTestCase() {
      // Use the malloc family for allocations
      Alloc_Reset();
      _fake_graph_context();
    }

    static void TearDownTestCase()
    {
        // Free fake graph context.
        GraphContext *gc = GraphContext_GetFromTLS();
        Schema_Free(gc->node_unified_schema);
        Schema_Free(gc->relation_unified_schema);
        free(gc);
    }

    static void _fake_graph_context() {
        /* Filter tree construction requires access to schemas, 
         * those inturn resides within graph context 
         * accessible via thread local storage, as such we're creating a 
         * fake graph context and placing it within thread local storage. */
        GraphContext *gc = (GraphContext*)malloc(sizeof(GraphContext));

        // No indicies.
        gc->index_count = 0;

        // Initialize the generic label and relation stores
        gc->node_unified_schema = Schema_New("ALL", GRAPH_NO_LABEL);
        gc->relation_unified_schema = Schema_New("ALL", GRAPH_NO_RELATION);
        
        int error = pthread_key_create(&_tlsGCKey, NULL);
        ASSERT_EQ(error, 0);
        pthread_setspecific(_tlsGCKey, gc);
    }

    AST* _build_ast(const char *query) {
        char *errMsg;
        AST **ast = ParseQuery(query, strlen(query), &errMsg);
        AST_NameAnonymousNodes(ast[0]);
        return ast[0];
    }

    FT_FilterNode* _build_simple_const_tree() {
        const char *query = "MATCH (me) WHERE me.age = 34 RETURN me";
        AST *ast = _build_ast(query);
        AST_FilterNode *root = ast->whereNode->filters;
        FT_FilterNode *tree = BuildFiltersTree(ast, root);
        return tree;
    }

    FT_FilterNode* _build_simple_varying_tree() {
        const char *query = "MATCH (me),(him) WHERE me.age > him.age RETURN me, him";
        AST *ast = _build_ast(query);
        AST_FilterNode *root = ast->whereNode->filters;
        FT_FilterNode *tree = BuildFiltersTree(ast, root);
        return tree;
    }

    FT_FilterNode* _build_cond_tree(int cond) {
        const char *query;
        if(cond == AND) query = "MATCH (me) WHERE me.age > 34 AND me.height <= 188 RETURN me";
        else  query = "MATCH (me) WHERE me.age > 34 OR me.height <= 188 RETURN me";

        AST *ast = _build_ast(query);
        AST_FilterNode *root = ast->whereNode->filters;
        FT_FilterNode *tree = BuildFiltersTree(ast, root);
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
        const char *query = "MATCH (me),(he),(she),(theirs) WHERE (me.age > 34 AND he.height <= 188) AND (she.age > 34 OR theirs.height <= 188) RETURN me, he, she, theirs";
        AST *ast = _build_ast(query);
        AST_FilterNode *root = ast->whereNode->filters;
        FT_FilterNode *tree = BuildFiltersTree(ast, root);
        return tree;
    }

    void _compareFilterTreePredicateNode(const FT_FilterNode *a, const FT_FilterNode *b) {
        ASSERT_EQ(a->t, b->t);
        ASSERT_EQ(a->t, FT_N_PRED);
        ASSERT_EQ(a->pred.op, b->pred.op);

        char *a_variable;
        char *b_variable;
        AR_EXP_ToString(a->pred.lhs, &a_variable);
        AR_EXP_ToString(b->pred.lhs, &b_variable);
        ASSERT_STREQ(a_variable, b_variable);

        free(a_variable);
        free(b_variable);

        AR_EXP_ToString(a->pred.rhs, &a_variable);
        AR_EXP_ToString(b->pred.rhs, &b_variable);
        ASSERT_STREQ(a_variable, b_variable);

        free(a_variable);
        free(b_variable);
    }

    void compareFilterTrees(const FT_FilterNode *a, const FT_FilterNode *b) {
        ASSERT_EQ(a->t, b->t);
        
        if(a->t == FT_N_PRED) {
            _compareFilterTreePredicateNode(a, b);
        } else {
            ASSERT_EQ(a->cond.op, b->cond.op);
            compareFilterTrees(a->cond.left, b->cond.left);
            compareFilterTrees(a->cond.right, b->cond.right);
        }
    }
};


TEST_F(FilterTreeTest, SubTrees) {
    FT_FilterNode *tree = _build_simple_const_tree();
    Vector *sub_trees = FilterTree_SubTrees(tree);
    ASSERT_EQ(Vector_Size(sub_trees), 1);

    FT_FilterNode *sub_tree;
    Vector_Get(sub_trees, 0, &sub_tree);
    compareFilterTrees(tree, sub_tree);

    Vector_Free(sub_trees);
    FilterTree_Free(tree);

    //------------------------------------------------------------------------------

    tree = _build_simple_varying_tree();
    sub_trees = FilterTree_SubTrees(tree);
    ASSERT_EQ(Vector_Size(sub_trees), 1);

    Vector_Get(sub_trees, 0, &sub_tree);
    compareFilterTrees(tree, sub_tree);

    Vector_Free(sub_trees);
    FilterTree_Free(tree);

    //------------------------------------------------------------------------------

    tree = _build_AND_cond_tree();
    sub_trees = FilterTree_SubTrees(tree);
    ASSERT_EQ(Vector_Size(sub_trees), 2);

    Vector_Get(sub_trees, 0, &sub_tree);    
    compareFilterTrees(tree->cond.left, sub_tree);
    
    Vector_Get(sub_trees, 1, &sub_tree);
    compareFilterTrees(tree->cond.right, sub_tree);

    Vector_Free(sub_trees);
    FilterTree_Free(tree);

    //------------------------------------------------------------------------------
    
    tree = _build_OR_cond_tree();
    sub_trees = FilterTree_SubTrees(tree);
    ASSERT_EQ(Vector_Size(sub_trees), 1);

    Vector_Get(sub_trees, 0, &sub_tree);
    compareFilterTrees(tree, sub_tree);

    Vector_Free(sub_trees);
    FilterTree_Free(tree);

    //------------------------------------------------------------------------------

    tree = _build_deep_tree();
    sub_trees = FilterTree_SubTrees(tree);
    ASSERT_EQ(Vector_Size(sub_trees), 3);

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
    ASSERT_EQ(Vector_Size(aliases), 4);
    
    char *alias;
    const char *expectation[4] = {"me", "he", "she", "theirs"};
    for(int i = 0; i < 4; i++) {
        const char *expected = expectation[i];
        int j = 0;
        for(; j < Vector_Size(aliases); j++) {
            Vector_Get(aliases, j, &alias);
            if(!strcmp(alias, expected)) break;
        }
        ASSERT_NE(j, 4);
    }

    /* Clean up. */
    for(int i = 0; i < Vector_Size(aliases); i++) {
        Vector_Get(aliases, i, &alias);
        free(alias);
    }
    Vector_Free(aliases);
    FilterTree_Free(tree);
}
