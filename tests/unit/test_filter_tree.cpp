/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include "../../src/query_ctx.h"
#include "../../src/util/arr.h"
#include "../../src/util/vector.h"
#include "../../src/util/rmalloc.h"
#include "../../src/filter_tree/filter_tree.h"
#include "../../src/ast/ast_build_filter_tree.h"
#include "../../src/arithmetic/funcs.h"

#ifdef __cplusplus
}
#endif

class FilterTreeTest: public ::testing::Test {
  protected:

	static void SetUpTestCase() {
		// Use the malloc family for allocations
		Alloc_Reset();
		_fake_graph_context();
	}

	static void TearDownTestCase() {
		// Free fake graph context.
		GraphContext *gc = QueryCtx_GetGraphCtx();
		free(gc);
	}

	static void _fake_graph_context() {
		/* Filter tree construction requires access to schemas,
		 * those inturn resides within graph context
		 * accessible via thread local storage, as such we're creating a
		 * fake graph context and placing it within thread local storage. */
		GraphContext *gc = (GraphContext *)malloc(sizeof(GraphContext));

		// No indicies.
		gc->index_count = 0;

		ASSERT_TRUE(QueryCtx_Init());
		QueryCtx_SetGraphCtx(gc);
		AR_RegisterFuncs();

	}

	AST *_build_ast(const char *query) {
		cypher_parse_result_t *parse_result = cypher_parse(query, NULL, NULL, CYPHER_PARSE_ONLY_STATEMENTS);
		AST *ast = AST_Build(parse_result);
		return ast;
	}

	FT_FilterNode *build_tree_from_query(const char *q) {
		AST *ast = _build_ast(q);
		FT_FilterNode *tree = AST_BuildFilterTree(ast);
		return tree;
	}

	FT_FilterNode *_build_simple_const_tree() {
		const char *q = "MATCH (me) WHERE me.age = 34 RETURN me";
		return build_tree_from_query(q);
	}

	FT_FilterNode *_build_simple_varying_tree() {
		const char *q = "MATCH (me),(him) WHERE me.age > him.age RETURN me, him";
		return build_tree_from_query(q);
	}

	FT_FilterNode *_build_cond_tree(int cond) {
		const char *q;
		if(cond == OP_AND) q = "MATCH (me) WHERE me.age > 34 AND me.height <= 188 RETURN me";
		else q = "MATCH (me) WHERE me.age > 34 OR me.height <= 188 RETURN me";
		return build_tree_from_query(q);
	}

	FT_FilterNode *_build_AND_cond_tree() {
		return _build_cond_tree(OP_AND);
	}

	FT_FilterNode *_build_OR_cond_tree() {
		return _build_cond_tree(OP_OR);
	}

	FT_FilterNode *_build_deep_tree() {
		/* Build a tree with an OP_AND at the root
		* OP_AND as a left child
		* OP_OR as a right child */
		const char *q =
			"MATCH (me),(he),(she),(theirs) WHERE (me.age > 34 AND he.height <= 188) AND (she.age > 34 OR theirs.height <= 188) RETURN me, he, she, theirs";
		return build_tree_from_query(q);
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

	FT_FilterNode *original_tree = _build_AND_cond_tree();
	tree = _build_AND_cond_tree(); // TODO memory leak
	sub_trees = FilterTree_SubTrees(tree);
	ASSERT_EQ(Vector_Size(sub_trees), 2);

	Vector_Get(sub_trees, 0, &sub_tree);
	compareFilterTrees(original_tree->cond.left, sub_tree);

	Vector_Get(sub_trees, 1, &sub_tree);
	compareFilterTrees(original_tree->cond.right, sub_tree);

	Vector_Free(sub_trees);
	// FilterTree_Free(tree);
	FilterTree_Free(original_tree);

	//------------------------------------------------------------------------------

	tree = _build_OR_cond_tree();
	sub_trees = FilterTree_SubTrees(tree);
	ASSERT_EQ(Vector_Size(sub_trees), 1);

	Vector_Get(sub_trees, 0, &sub_tree);
	compareFilterTrees(tree, sub_tree);

	Vector_Free(sub_trees);
	FilterTree_Free(tree);

	//------------------------------------------------------------------------------

	original_tree = _build_deep_tree();
	tree = _build_deep_tree(); // TODO memory leak
	sub_trees = FilterTree_SubTrees(tree);
	ASSERT_EQ(Vector_Size(sub_trees), 3);

	Vector_Get(sub_trees, 0, &sub_tree);
	compareFilterTrees(original_tree->cond.left->cond.left, sub_tree);

	Vector_Get(sub_trees, 1, &sub_tree);
	compareFilterTrees(original_tree->cond.left->cond.right, sub_tree);

	Vector_Get(sub_trees, 2, &sub_tree);
	compareFilterTrees(original_tree->cond.right, sub_tree);

	Vector_Free(sub_trees);
	FilterTree_Free(original_tree);
}

TEST_F(FilterTreeTest, CollectModified) {
	FT_FilterNode *tree = _build_deep_tree();
	rax *aliases = FilterTree_CollectModified(tree);
	ASSERT_EQ(raxSize(aliases), 4);

	char *expectation[4] = {"me", "he", "she", "theirs"};
	for(int i = 0; i < 4; i++) {
		char *expected = expectation[i];
		ASSERT_NE(raxFind(aliases, (unsigned char *)expected, strlen(expected)), raxNotFound);
	}

	/* Clean up. */
	raxFree(aliases);
	FilterTree_Free(tree);
}

TEST_F(FilterTreeTest, NOTReduction) {
	const char *filters[6] {
		"MATCH (n) WHERE NOT n.v = 1 RETURN n",
		"MATCH (n) WHERE NOT NOT n.v = 1 RETURN n",
		"MATCH (n) WHERE NOT (n.v > 5 AND n.v < 20) RETURN n",
		"MATCH (n) WHERE NOT (n.v <= 5 OR n.v <> 20) RETURN n",
		"MATCH (n) WHERE NOT( NOT( NOT( n.v >= 1 AND (n.v < 5 OR n.v <> 3) ) ) ) RETURN n",
		"MATCH (n) WHERE n.v = 2 AND NOT n.x > 4 RETURN n",
	};

	const char *expected[6] {
		"MATCH (n) WHERE n.v <> 1 RETURN n",
		"MATCH (n) WHERE n.v = 1 RETURN n",
		"MATCH (n) WHERE n.v <= 5 OR n.v >= 20 RETURN n",
		"MATCH (n) WHERE n.v > 5 AND n.v = 20 RETURN n",
		"MATCH (n) WHERE (n.v < 1 OR (n.v >= 5 AND n.v = 3)) RETURN n",
		"MATCH (n) WHERE n.v = 2 AND n.x <= 4 RETURN n",
	};

	for(int i = 0; i < 6; i++) {
		const char *actual = filters[i];
		const char *expected_q = expected[i];

		FT_FilterNode *actual_tree = build_tree_from_query(actual);
		FT_FilterNode *expected_tree = build_tree_from_query(expected_q);

		compareFilterTrees(actual_tree, expected_tree);

		FilterTree_Free(actual_tree);
		FilterTree_Free(expected_tree);
	}
}

<<<<<<< HEAD
TEST_F(FilterTreeTest, InvalidTree) {
	/* MATCH (u) where u.v NOT NULL RETURN u
	 * is an invalid query,
	 * should have been:
	 * MATCH (u) where u.v IS NOT NULL RETURN u */
	const char *query = "MATCH (u) where u.v NOT NULL RETURN u";
	FT_FilterNode *tree = build_tree_from_query(query);
	ASSERT_TRUE(tree == NULL);
}

=======
>>>>>>> wip
TEST_F(FilterTreeTest, ContainsFunc) {
	bool found = false;
	FT_FilterNode *node = NULL;
	const char *q = "MATCH (n) WHERE tolower(n.name) = 'alex' RETURN n";
	FT_FilterNode *tree = build_tree_from_query(q);

	found = FilterTree_containsFunc(tree, "tolower", &node);
	ASSERT_TRUE(found);
	ASSERT_TRUE(node != NULL);

	node = NULL;
	found = FilterTree_containsFunc(tree, "toupper", &node);
	ASSERT_FALSE(found);
	ASSERT_TRUE(node == NULL);

	FilterTree_Free(tree);
	//------------------------------------------------------------------------------

	q = "MATCH (n) WHERE tolower(toupper(n.name)) = 'alex' RETURN n";
	tree = build_tree_from_query(q);

	found = FilterTree_containsFunc(tree, "tolower", &node);
	ASSERT_TRUE(found);
	ASSERT_TRUE(node != NULL);

	node = NULL;
	found = FilterTree_containsFunc(tree, "toupper", &node);
	ASSERT_TRUE(found);
	ASSERT_TRUE(node != NULL);

	FilterTree_Free(tree);
}

