/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "../../src/util/rmalloc.h"
#include "../../src/query_ctx.h"
#include "../../src/arithmetic/funcs.h"
#include "../../src/graph/query_graph.h"
#include "../../src/filter_tree/filter_tree.h"
#include "../../src/ast/ast_build_filter_tree.h"
#include "../../src/arithmetic/algebraic_expression.h"
#include "../../src/execution_plan/optimizations/optimizations.h"

#ifdef __cplusplus
}
#endif

class TraversalOrderingTest: public ::testing::Test {
  protected:
	static void SetUpTestCase() {
		// Use the malloc family for allocations
		Alloc_Reset();
		_fake_graph_context();
	}

	static void TearDownTestCase() {
	}

	static void _fake_graph_context() {
		/* Filter tree construction requires access to schemas,
		 * those inturn resides within graph context
		 * accessible via thread local storage, as such we're creating a
		 * fake graph context and placing it within thread local storage. */
		GraphContext *gc = (GraphContext *)calloc(1, sizeof(GraphContext));
		gc->attributes = raxNew();
		pthread_rwlock_init(&gc->_attribute_rwlock, NULL);

		// Prepare thread-local variables
		ASSERT_TRUE(QueryCtx_Init());
		QueryCtx_SetGraphCtx(gc);
		AR_RegisterFuncs();
	}

	AST *_build_ast(const char *query) {
		cypher_parse_result_t *parse_result = cypher_parse(query, NULL, NULL, CYPHER_PARSE_ONLY_STATEMENTS);
		AST *ast = AST_Build(parse_result);
		return ast;
	}

	FT_FilterNode *build_filter_tree_from_query(const char *query) {
		AST *ast = _build_ast(query);
		return AST_BuildFilterTree(ast);
	}
};

TEST_F(TraversalOrderingTest, FilterFirst) {
	/* Given the ordered (left to right) set of algebraic expression:
	 * { [AB], [BC], [CD] }
	 * Which represents the traversal:
	 * (A)->(B)->(C)->(D)
	 * And a set of filters:
	 * C.V = X.
	 *
	 * We can reorder this set in such away
	 * that filters are applied as early as possible.
	 *
	 * Here are all of the possible permutations of the set
	 * in which the filter is applied at the earliest step:
	 * { [CB], [BA], [CD] } (D)<-(C)->(B)->(A) (2 transposes)
	 * { [CB], [CD], [BA] } (D)<-(C)->(B)->(A) (2 transposes)
	 * { [CD], [CB], [BA] } (A)<-(B)<-(C)->(D) (2 transposes) */

	FT_FilterNode *filters;
	QGNode *A = QGNode_New("A");
	QGNode *B = QGNode_New("B");
	QGNode *C = QGNode_New("C");
	QGNode *D = QGNode_New("D");

	QGEdge *AB = QGEdge_New(A, B, "E", "AB");
	QGEdge *BC = QGEdge_New(B, C, "E", "BC");
	QGEdge *CD = QGEdge_New(C, D, "E", "CD");

	QueryGraph *qg = QueryGraph_New(4, 3);

	QueryGraph_AddNode(qg, A);
	QueryGraph_AddNode(qg, B);
	QueryGraph_AddNode(qg, C);
	QueryGraph_AddNode(qg, D);
	QueryGraph_ConnectNodes(qg, A, B, AB);
	QueryGraph_ConnectNodes(qg, B, C, BC);
	QueryGraph_ConnectNodes(qg, C, D, CD);

	AlgebraicExpression *set[3];
	AlgebraicExpression *ExpAB = AlgebraicExpression_NewOperand(GrB_NULL, false, "A", "B", NULL, NULL);
	AlgebraicExpression *ExpBC = AlgebraicExpression_NewOperand(GrB_NULL, false, "B", "C", NULL, NULL);
	AlgebraicExpression *ExpCD = AlgebraicExpression_NewOperand(GrB_NULL, false, "C", "D", NULL, NULL);

	// { [AB], [BC], [CD] }
	set[0] = ExpAB;
	set[1] = ExpBC;
	set[2] = ExpCD;

	filters = build_filter_tree_from_query(
				  "MATCH (A)-[]->(B)-[]->(C)-[]->(D) WHERE A.val = 1 RETURN *");

	orderExpressions(qg, set, 3, filters, NULL);
	ASSERT_EQ(set[0], ExpAB);
	ASSERT_EQ(set[1], ExpBC);
	ASSERT_EQ(set[2], ExpCD);

	FilterTree_Free(filters);

	// { [AB], [BC], [CD] }
	set[0] = ExpAB;
	set[1] = ExpBC;
	set[2] = ExpCD;

	filters = build_filter_tree_from_query("MATCH (A)-[]->(B)-[]->(C)-[]->(D) WHERE B.val = 1 RETURN *");

	orderExpressions(qg, set, 3, filters, NULL);

	ASSERT_STREQ(AlgebraicExpression_Source(set[0]), "B");

	FilterTree_Free(filters);

	// { [AB], [BC], [CD] }
	set[0] = ExpAB;
	set[1] = ExpBC;
	set[2] = ExpCD;

	filters = build_filter_tree_from_query("MATCH (A)-[]->(B)-[]->(C)-[]->(D) WHERE C.val = 1 RETURN *");

	orderExpressions(qg, set, 3, filters, NULL);
	ASSERT_STREQ(AlgebraicExpression_Source(set[0]), "C");

	FilterTree_Free(filters);

	// { [AB], [BC], [CD] }
	set[0] = ExpAB;
	set[1] = ExpBC;
	set[2] = ExpCD;

	filters = build_filter_tree_from_query("MATCH (A)-[]->(B)-[]->(C)-[]->(D) WHERE D.val = 1 RETURN *");

	orderExpressions(qg, set, 3, filters, NULL);

	ASSERT_STREQ(AlgebraicExpression_Source(set[0]), "D");

	// Clean up.
	FilterTree_Free(filters);
	AlgebraicExpression_Free(ExpAB);
	AlgebraicExpression_Free(ExpBC);
	AlgebraicExpression_Free(ExpCD);
	QueryGraph_Free(qg);
}

TEST_F(TraversalOrderingTest, ValidateScoring) {
	/* Given the set of algebraic expressions:
	 * { [AB], [BC], [BD] }
	 * We represent the traversal:
	 * (A:L {v: 1})->(B)->(C), (B)->(D:L {v: 1})
	 * The optimal order of traversals should always be:
	 * { [AB], [BD], [BC] }
	 * Validate this for all input permutations.
	 */

	FT_FilterNode *filters;
	QGNode *A = QGNode_New("A");
	QGNode *B = QGNode_New("B");
	QGNode *C = QGNode_New("C");
	QGNode *D = QGNode_New("D");
	A->label = "L";
	D->label = "L";

	QGEdge *AB = QGEdge_New(A, B, "E", "AB");
	QGEdge *BC = QGEdge_New(B, C, "E", "BC");
	QGEdge *BD = QGEdge_New(B, D, "E", "BD");

	QueryGraph *qg = QueryGraph_New(4, 3);

	QueryGraph_AddNode(qg, A);
	QueryGraph_AddNode(qg, B);
	QueryGraph_AddNode(qg, C);
	QueryGraph_AddNode(qg, D);
	QueryGraph_ConnectNodes(qg, A, B, AB);
	QueryGraph_ConnectNodes(qg, B, C, BC);
	QueryGraph_ConnectNodes(qg, C, D, BD);

	AlgebraicExpression *set[3];
	AlgebraicExpression *ExpAB = AlgebraicExpression_NewOperand(GrB_NULL, false, "A", "B", NULL, "L");
	AlgebraicExpression *ExpBC = AlgebraicExpression_NewOperand(GrB_NULL, false, "B", "C", NULL, NULL);
	AlgebraicExpression *ExpBD = AlgebraicExpression_NewOperand(GrB_NULL, false, "B", "D", NULL, "L");

	filters = build_filter_tree_from_query(
				  "MATCH (A)-[]->(B)-[]->(C)-[]->(D), (B)-[]->(E) WHERE A.val = 1 AND D.val = 1 RETURN *");

	// Test every permutation of the set.
	set[0] = ExpAB;
	set[1] = ExpBC;
	set[2] = ExpBD;
	orderExpressions(qg, set, 3, filters, NULL);
	ASSERT_EQ(set[0], ExpAB);
	ASSERT_EQ(set[1], ExpBD);
	ASSERT_EQ(set[2], ExpBC);

	set[0] = ExpAB;
	set[1] = ExpBD;
	set[2] = ExpBC;
	orderExpressions(qg, set, 3, filters, NULL);
	ASSERT_EQ(set[0], ExpAB);
	ASSERT_EQ(set[1], ExpBD);
	ASSERT_EQ(set[2], ExpBC);

	set[0] = ExpBC;
	set[1] = ExpBD;
	set[2] = ExpAB;
	orderExpressions(qg, set, 3, filters, NULL);
	ASSERT_EQ(set[0], ExpAB);
	ASSERT_EQ(set[1], ExpBD);
	ASSERT_EQ(set[2], ExpBC);

	set[0] = ExpBC;
	set[1] = ExpAB;
	set[2] = ExpBD;
	orderExpressions(qg, set, 3, filters, NULL);
	ASSERT_EQ(set[0], ExpAB);
	ASSERT_EQ(set[1], ExpBD);
	ASSERT_EQ(set[2], ExpBC);

	set[0] = ExpBD;
	set[1] = ExpAB;
	set[2] = ExpBC;
	orderExpressions(qg, set, 3, filters, NULL);
	ASSERT_EQ(set[0], ExpAB);
	ASSERT_EQ(set[1], ExpBD);
	ASSERT_EQ(set[2], ExpBC);

	set[0] = ExpBD;
	set[1] = ExpBC;
	set[2] = ExpAB;
	orderExpressions(qg, set, 3, filters, NULL);
	ASSERT_EQ(set[0], ExpAB);
	ASSERT_EQ(set[1], ExpBD);
	ASSERT_EQ(set[2], ExpBC);

	FilterTree_Free(filters);

	// Clean up.
	AlgebraicExpression_Free(ExpAB);
	AlgebraicExpression_Free(ExpBC);
	AlgebraicExpression_Free(ExpBD);
	QueryGraph_Free(qg);
}

