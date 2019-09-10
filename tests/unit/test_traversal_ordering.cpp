/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "../../src/util/rmalloc.h"
#include "../../src/query_ctx.h"
#include "../../src/graph/query_graph.h"
#include "../../src/filter_tree/filter_tree.h"
#include "../../src/ast/ast_build_filter_tree.h"
#include "../../src/arithmetic/algebraic_expression.h"
#include "../../src/execution_plan/optimizations/traverse_order.h"

#ifdef __cplusplus
}
#endif

class TraversalOrderingTest: public ::testing::Test {
  protected:
	static void SetUpTestCase() {
		// Use the malloc family for allocations
		Alloc_Reset();

		// Prepare thread-local variables
		QueryCtx_Init();
		QueryCtx_Begin();
	}

	static void TearDownTestCase() {
	}

	AST *_build_ast(const char *query) {
		cypher_parse_result_t *parse_result = cypher_parse(query, NULL, NULL, CYPHER_PARSE_ONLY_STATEMENTS);
		AST *ast = AST_Build(parse_result);
		return ast;
	}

	FT_FilterNode *build_filter_tree_from_query(RecordMap *map, const char *query) {
		AST *ast = _build_ast(query);
		return AST_BuildFilterTree(ast, map);
	}
};

TEST_F(TraversalOrderingTest, TransposeFree) {
	RecordMap *map = RecordMap_New();
	/* Given the ordered (left to right) set of algebraic expression:
	 * { [CD], [BC], [AB] }
	 * Which represents the traversal:
	 * (A)->(B)->(C)->(D)
	 * If we choose to start at C
	 * we can continue to D.
	 * Retract from C to B
	 * Retract from B to A.
	 * Overall we've performed 2 transposes:
	 * (A)<-(B) and (B)<-(C).
	 *
	 * We can reorder this set in such away
	 * that we won't perform any transposes.
	 *
	 * Here are all of the possible permutations of the set:
	 * { [AB], [BC], [CD] } (A)->(B)->(C)->(D)
	 * { [AB], [CD], [BC] } Invalid arrangement.
	 * { [BC], [AB], [CD] } (A)<-(B)->(C)->(D)
	 * { [BC], [CD], [AB] } (A)<-(B)->(C)->(D)
	 * { [CD], [AB], [BC] } Invalid arrangement.
	 * { [CD], [BC], [AB] } (A)<-(B)<-(C)->(D)
	 *
	 * Arrangement { [AB], [BC], [CD] }
	 * Is the only one that doesn't requires any transposes. */

	uint id = 0;
	QGNode *A = QGNode_New(NULL, "A", id++);
	QGNode *B = QGNode_New(NULL, "B", id++);
	QGNode *C = QGNode_New(NULL, "C", id++);
	QGNode *D = QGNode_New(NULL, "D", id++);

	QGEdge *AB = QGEdge_New(A, B, "E", "AB", id++);
	QGEdge *BC = QGEdge_New(B, C, "E", "BC", id++);
	QGEdge *CD = QGEdge_New(C, D, "E", "CD", id++);

	QueryGraph *qg = QueryGraph_New(4, 3);

	QueryGraph_AddNode(qg, A);
	QueryGraph_AddNode(qg, B);
	QueryGraph_AddNode(qg, C);
	QueryGraph_AddNode(qg, D);
	QueryGraph_ConnectNodes(qg, A, B, AB);
	QueryGraph_ConnectNodes(qg, B, C, BC);
	QueryGraph_ConnectNodes(qg, C, D, CD);

	AlgebraicExpression *set[3];
	AlgebraicExpression *ExpAB = AlgebraicExpression_Empty();
	AlgebraicExpression *ExpBC = AlgebraicExpression_Empty();
	AlgebraicExpression *ExpCD = AlgebraicExpression_Empty();

	AlgebraicExpression_AppendTerm(ExpAB, NULL, false, false, false);
	AlgebraicExpression_AppendTerm(ExpBC, NULL, false, false, false);
	AlgebraicExpression_AppendTerm(ExpCD, NULL, false, false, false);

	ExpAB->src_node = A;
	ExpAB->dest_node = B;
	ExpBC->src_node = B;
	ExpBC->dest_node = C;
	ExpCD->src_node = C;
	ExpCD->dest_node = D;

	// { [CD], [BC], [AB] }
	set[0] = ExpCD;
	set[1] = ExpBC;
	set[2] = ExpAB;

	orderExpressions(set, 3, map, NULL);
	ASSERT_EQ(set[0], ExpAB);
	ASSERT_EQ(set[1], ExpBC);
	ASSERT_EQ(set[2], ExpCD);

	// { [AB], [BC], [CD] }
	set[0] = ExpAB;
	set[1] = ExpBC;
	set[2] = ExpCD;
	orderExpressions(set, 3, map, NULL);
	ASSERT_EQ(set[0], ExpAB);
	ASSERT_EQ(set[1], ExpBC);
	ASSERT_EQ(set[2], ExpCD);

	// { [AB], [CD], [BC] }
	set[0] = ExpAB;
	set[1] = ExpCD;
	set[2] = ExpBC;
	orderExpressions(set, 3, map, NULL);
	ASSERT_EQ(set[0], ExpAB);
	ASSERT_EQ(set[1], ExpBC);
	ASSERT_EQ(set[2], ExpCD);

	// { [BC], [AB], [CD] }
	set[0] = ExpBC;
	set[1] = ExpAB;
	set[2] = ExpCD;
	orderExpressions(set, 3, map, NULL);
	ASSERT_EQ(set[0], ExpAB);
	ASSERT_EQ(set[1], ExpBC);
	ASSERT_EQ(set[2], ExpCD);

	// { [BC], [CD], [AB] }
	set[0] = ExpBC;
	set[1] = ExpCD;
	set[2] = ExpAB;
	orderExpressions(set, 3, map, NULL);
	ASSERT_EQ(set[0], ExpAB);
	ASSERT_EQ(set[1], ExpBC);
	ASSERT_EQ(set[2], ExpCD);

	// { [CD], [AB], [BC] }
	set[0] = ExpCD;
	set[1] = ExpAB;
	set[2] = ExpBC;
	orderExpressions(set, 3, map, NULL);
	ASSERT_EQ(set[0], ExpAB);
	ASSERT_EQ(set[1], ExpBC);
	ASSERT_EQ(set[2], ExpCD);

	// Clean up.
	AlgebraicExpression_Free(ExpAB);
	AlgebraicExpression_Free(ExpBC);
	AlgebraicExpression_Free(ExpCD);
	QueryGraph_Free(qg);
}

TEST_F(TraversalOrderingTest, FilterFirst) {
	RecordMap *map = RecordMap_New();
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

	uint id = 0;
	FT_FilterNode *filters;
	QGNode *A = QGNode_New(NULL, "A", id++);
	QGNode *B = QGNode_New(NULL, "B", id++);
	QGNode *C = QGNode_New(NULL, "C", id++);
	QGNode *D = QGNode_New(NULL, "D", id++);

	QGEdge *AB = QGEdge_New(A, B, "E", "AB", id++);
	QGEdge *BC = QGEdge_New(B, C, "E", "BC", id++);
	QGEdge *CD = QGEdge_New(C, D, "E", "CD", id++);

	QueryGraph *qg = QueryGraph_New(4, 3);

	QueryGraph_AddNode(qg, A);
	QueryGraph_AddNode(qg, B);
	QueryGraph_AddNode(qg, C);
	QueryGraph_AddNode(qg, D);
	QueryGraph_ConnectNodes(qg, A, B, AB);
	QueryGraph_ConnectNodes(qg, B, C, BC);
	QueryGraph_ConnectNodes(qg, C, D, CD);

	AlgebraicExpression *set[3];
	AlgebraicExpression *ExpAB = AlgebraicExpression_Empty();
	AlgebraicExpression *ExpBC = AlgebraicExpression_Empty();
	AlgebraicExpression *ExpCD = AlgebraicExpression_Empty();

	AlgebraicExpression_AppendTerm(ExpAB, NULL, false, false, false);
	AlgebraicExpression_AppendTerm(ExpBC, NULL, false, false, false);
	AlgebraicExpression_AppendTerm(ExpCD, NULL, false, false, false);

	ExpAB->src_node = A;
	ExpAB->dest_node = B;
	ExpBC->src_node = B;
	ExpBC->dest_node = C;
	ExpCD->src_node = C;
	ExpCD->dest_node = D;

	// { [AB], [BC], [CD] }
	set[0] = ExpAB;
	set[1] = ExpBC;
	set[2] = ExpCD;

	filters = build_filter_tree_from_query(map,
										   "MATCH (A)-[]->(B)-[]->(C)-[]->(D) WHERE A.val = 1 RETURN *");

	orderExpressions(set, 3, map, filters);
	ASSERT_EQ(set[0], ExpAB);
	ASSERT_EQ(set[1], ExpBC);
	ASSERT_EQ(set[2], ExpCD);

	FilterTree_Free(filters);

	// { [AB], [BC], [CD] }
	set[0] = ExpAB;
	set[1] = ExpBC;
	set[2] = ExpCD;

	filters = build_filter_tree_from_query(map,
										   "MATCH (A)-[]->(B)-[]->(C)-[]->(D) WHERE B.val = 1 RETURN *");

	orderExpressions(set, 3, map, filters);
	ASSERT_TRUE(set[0] == ExpAB || set[0] == ExpBC);

	FilterTree_Free(filters);

	// { [AB], [BC], [CD] }
	set[0] = ExpAB;
	set[1] = ExpBC;
	set[2] = ExpCD;

	filters = build_filter_tree_from_query(map,
										   "MATCH (A)-[]->(B)-[]->(C)-[]->(D) WHERE C.val = 1 RETURN *");

	orderExpressions(set, 3, map, filters);
	ASSERT_TRUE(set[0] == ExpBC || set[0] == ExpCD);

	FilterTree_Free(filters);

	// { [AB], [BC], [CD] }
	set[0] = ExpAB;
	set[1] = ExpBC;
	set[2] = ExpCD;

	filters = build_filter_tree_from_query(map,
										   "MATCH (A)-[]->(B)-[]->(C)-[]->(D) WHERE D.val = 1 RETURN *");

	orderExpressions(set, 3, map, filters);

	ASSERT_EQ(set[0], ExpCD);
	ASSERT_EQ(set[1], ExpBC);
	ASSERT_EQ(set[2], ExpAB);

	// Clean up.
	FilterTree_Free(filters);
	RecordMap_Free(map);
	AlgebraicExpression_Free(ExpAB);
	AlgebraicExpression_Free(ExpBC);
	AlgebraicExpression_Free(ExpCD);
	QueryGraph_Free(qg);
}

